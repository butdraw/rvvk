#include "include/rvvk.h"
#include "include/elf.h"

/*
 * Verify the portions of EHDR within E_IDENT for the target.
 */
static int elf_check_ehdr_ident(elf_ehdr_t *ehdr)
{
    return (ehdr->e_ident[EI_MAG0] == ELFMAG0
            && ehdr->e_ident[EI_MAG1] == ELFMAG1
            && ehdr->e_ident[EI_MAG2] == ELFMAG2
            && ehdr->e_ident[EI_MAG3] == ELFMAG3
            && ehdr->e_ident[EI_CLASS] == ELF_CLASS
            && ehdr->e_ident[EI_DATA] == ELF_DATA
            && ehdr->e_ident[EI_VERSION] == EV_CURRENT);
}

/*
 * Verify the portions of EHDR outside of E_IDENT for the target.
 */
static int elf_check_ehdr1(elf_ehdr_t *ehdr)
{
    return ((ehdr->e_type == ET_EXEC || ehdr->e_type == ET_DYN)
            && ehdr->e_machine == EM_ARCH
            && ehdr->e_version == EV_CURRENT
            && ehdr->e_ehsize == sizeof(elf_ehdr_t)
            && ehdr->e_phentsize == sizeof(elf_phdr_t));
}

static int elf_check_ehdr(elf_ehdr_t *ehdr, errinfo_t *errp)
{
    /* Check ELF signature */
    if (!elf_check_ehdr_ident(ehdr)) {
        get_errinfo(errp, "Failed to check E_IDENT of EHDR.");
        return 0;
    }

    /* Check the portions of EHDR outside of E_IDENT */
    if (!elf_check_ehdr1(ehdr)) {
        get_errinfo(errp, "Failed to check the portions of EHDR outside of E_IDENT.");
        return 0;
    }

    return 1;
}

static elf_phdr_t *load_elf_phdrs(const elf_ehdr_t *ehdr, int fd, errinfo_t *errp)
{
    elf_phdr_t *phdr;
    unsigned int size, retval;

    size = ehdr->e_phnum * sizeof(*phdr);

    /* Sanity check the number of program headers and their total size. */
    if (size == 0 || size > 65536 || size > TARGET_ELF_MIN_ALIGN) {
        get_errinfo(errp, "Inappropriate total size of file program headers.");
        return NULL;
    }

    phdr = malloc(size);
    if (!phdr) {
        get_errinfo(errp, "Can't allocate extra memory.");
        return NULL;
    }

    retval = pread(fd, phdr, size, ehdr->e_phoff);
    if (retval != size) {
        get_errinfo(errp, "Incompleted read of file program header.");
        return NULL;
    }
    return phdr;
}

int zero_bss(target_ulong elf_bss, target_ulong elf_brk, int prot, errinfo_t *errp)
{
    void *p;
    uintptr_t host_start, host_end, host_start_aligned;
    elf_brk = TARGET_PAGE_ALIGN(elf_brk);

    host_start = (uintptr_t) g2h(elf_bss);
    host_end = (uintptr_t) g2h(elf_brk);
    host_start_aligned = HOST_PAGE_ALIGN(host_start);

    if (host_start_aligned < host_end) {
        p = host_mmap((void *)host_start_aligned, host_end - host_start_aligned, prot,
#ifndef __STRICT_ANSI__
                MAP_ANONYMOUS |
#endif
                MAP_FIXED | MAP_PRIVATE, -1, 0, errp);
        if (p == MAP_FAILED) {
            return -1;
        }
    }

    if (host_start < host_start_aligned) {
        memset((void *)host_start, 0, host_start_aligned - host_start);
    }

    return 0;
}

/*
 * present useful information to the program by shovelling it onto the new
 * process's stack
 */
static int create_elf_tables(struct linux_binprm *bprm, errinfo_t *errp)
{
    int i, j, n;
    char **p;
    target_ulong top = bprm->p;
    target_ulong *ptrs =
        malloc((1 + bprm->envc + 1 + bprm->argc + 1) * sizeof(target_ulong));
    if (!ptrs) {
        get_syserr(errp);
        return -1;
    }

    /* Transfer strings pointed to envp[i] to stack.  */
    j = 0;
    ptrs[j++] = 0;
    for (i = bprm->envc - 1, p = bprm->envp + i; i >= 0; i--, p--) {
        n = strlen(*p) + 1;
        top -= n;
        ptrs[j++] = top;
        if (target_memory_copy(top, *p, n, errp) < 0) {
            return -1;
        }
    }

    /* Transfer strings pointed to argv[i] (exclude argv[0]) to stack.  */
    ptrs[j++] = 0;
    for (i = bprm->argc - 1, p = bprm->argv + i; i > 0; i--, p--) {
        n = strlen(*p) + 1;
        top -= n;
        ptrs[j++] = top;
        if (target_memory_copy(top, *p, n, errp) < 0) {
            return -1;
        }
    }

    /* Transfer string pointed to argv[0] to stack.  */
    n = strlen(bprm->filename) + 1;
    top -= n;
    ptrs[j++] = top;
    if (target_memory_copy(top, bprm->filename, n, errp) < 0) {
        return -1;
    }
    ptrs[j++] = bprm->argc;

    /* AUXV: Null */
    top = ROUND_DOWN(top, sizeof(target_ulong));
    /* auxv end */
    top -= sizeof(target_ulong);
    /* elf_auxv_t */
    top -= sizeof(elf_auxv_t);

    /* Transfer addresses of all strings to stack.  */
    // top = ROUND_DOWN(top, sizeof(target_ulong));
    for (i = 0; i < j; i++) {
        top -= sizeof(target_ulong);
        if (target_memory_copy(top, &ptrs[i], sizeof(target_ulong), errp) < 0) {
            return -1;
        }
    }

    bprm->p = top;

    free(ptrs);
    return 0;
}

static int load_elf_binary(struct linux_binprm *bprm, struct image_info *info,
                           errinfo_t *errp)
{
    int i;
    elf_ehdr_t *ehdr;
    elf_phdr_t *phdr, *eppnt;
    target_ulong load_addr, load_bias, loaddr, hiaddr, addr, brk_aligned;
    target_ulong vaddr, vaddr_po, vaddr_ps, vaddr_ef, vaddr_em, vaddr_len;
    target_ulong stack_limit, stack_size, guard;
    int prot;

    ehdr = (elf_ehdr_t *)bprm->buf;
    if (!elf_check_ehdr(ehdr, errp)) {
        return -1;
    }

    phdr = load_elf_phdrs(ehdr, bprm->execfd, errp);
    if (!phdr) {
        return -1;
    }

    info->nsegs = 0;
    info->alignment = 0;
    /*
     * Find the maximum size of the image and allocate an appropriate
     * amount of memory to handle that.  Locate the interpreter, if any.
     */
    loaddr = -1;
    hiaddr = 0;
    for (i = 0, eppnt = phdr; i < ehdr->e_phnum; i++, eppnt++) {
        if (eppnt->p_type == PT_LOAD) {
            addr = eppnt->p_vaddr - eppnt->p_offset;
            if (addr < loaddr) {
                loaddr = addr;
            }
            addr = eppnt->p_vaddr + eppnt->p_memsz - 1;
            if (addr > hiaddr) {
                hiaddr = addr;
            }
            info->nsegs++;
            info->alignment |= eppnt->p_align;
        } else if (eppnt->p_type == PT_INTERP) {
            /*
             * TODO: prepare to load a dynamic binary
             */
            continue;
        } else if (eppnt->p_type == PT_GNU_STACK) {
            info->exec_stack = eppnt->p_flags & PF_X;
        }
    }

    info->reserve_brk = 32 * MiB; /* heap? */
    hiaddr += info->reserve_brk;

    /*
     * Reserve address space for all of this.
     *
     * TODO: Mapping a block of address space for dynamic binary.
     *       For now, we failed.
     */
    load_addr = target_mmap(loaddr, (size_t)hiaddr - loaddr + 1, PROT_NONE,
#ifndef __STRICT_ANSI__
            MAP_NORESERVE | MAP_ANONYMOUS |
#endif
            MAP_PRIVATE | (ehdr->e_type == ET_EXEC ? MAP_FIXED : 0),
            -1, 0, errp);
    if (load_addr == (target_ulong) -1) {
        return -1;
    }
    /* for dynamic loading */
    load_bias = load_addr - loaddr;

    info->load_bias = load_bias;
    info->code_offset = load_bias;
    info->data_offset = load_bias;
    info->load_addr = load_addr;
    info->entry = ehdr->e_entry + load_bias;
    info->start_code = -1;
    info->end_code = 0;
    info->start_data = -1;
    info->end_data = 0;
    info->brk = 0;
    info->elf_flags = ehdr->e_flags;

    for (i = 0, eppnt = phdr; i < ehdr->e_phnum; i++, eppnt++) {
        if (eppnt->p_type != PT_LOAD) {
            continue;
        }

        prot = 0;
        if (eppnt->p_flags & PF_R) {
            prot |= PROT_READ;
        }
        if (eppnt->p_flags & PF_W) {
            prot |= PROT_WRITE;
        }
        if (eppnt->p_flags & PF_X) {
            prot |= PROT_EXEC;
            prot |= PROT_READ;
        }

        vaddr = load_bias + eppnt->p_vaddr;
        vaddr_po = TARGET_ELF_PAGEOFFSET(vaddr);
        vaddr_ps = TARGET_ELF_PAGESTART(vaddr);

        vaddr_ef = vaddr + eppnt->p_filesz;
        vaddr_em = vaddr + eppnt->p_memsz;

        /*
         * Some segments may be completely empty, with a non-zero p_memsz
         * but no backing file segment.
         */
        if (eppnt->p_filesz != 0) {
            vaddr_len = TARGET_ELF_PAGELENGTH(eppnt->p_filesz + vaddr_po);
            addr = target_mmap(vaddr_ps, vaddr_len, prot, MAP_PRIVATE | MAP_FIXED,
                    bprm->execfd, eppnt->p_offset - vaddr_po, errp);

            if (addr == -1) {
                return -1;
            }

            /*
             * If the load segment requests extra zeros (e.g. bss), map it.
             */
            if (eppnt->p_filesz < eppnt->p_memsz) {
                if(zero_bss(vaddr_ef, vaddr_em, prot, errp) < 0) {
                    return -1;
                }
            }
        } else if (eppnt->p_memsz != 0) {
            vaddr_len = TARGET_ELF_PAGELENGTH(eppnt->p_memsz + vaddr_po);
            addr = target_mmap(vaddr_ps, vaddr_len, prot,
#ifndef __STRICT_ANSI__
                    MAP_ANONYMOUS |
#endif
                    MAP_PRIVATE | MAP_FIXED, -1, 0, errp);

            if (addr == -1) {
                return -1;
            }
        }

        /* Find the full program boundaries.  */
        if (prot & PROT_EXEC) {
            if (vaddr < info->start_code) {
                info->start_code = vaddr;
            }
            if (vaddr_ef > info->end_code) {
                info->end_code = vaddr_ef;
            }
        }
        if (!(prot & PROT_READ)) {
            get_errinfo(errp, "Load a unreadable segment, not supported.");
            return -1;
        }
        if (vaddr < info->start_data) {
            info->start_data = vaddr;
        }
        if (vaddr_ef > info->end_data) {
            info->end_data = vaddr_ef;
        }
        if (vaddr_em > info->brk) {
            info->brk = vaddr_em;
        }
    }

    /* static heap */
    prot = PROT_READ | PROT_WRITE;
    brk_aligned = ROUND_UP(info->brk, TARGET_PAGE_SIZE);
    brk_aligned = target_mmap(brk_aligned, (size_t)hiaddr - brk_aligned + 1, prot,
#ifndef __STRICT_ANSI__
            MAP_NORESERVE | MAP_ANONYMOUS |
#endif
            MAP_PRIVATE | (ehdr->e_type == ET_EXEC ? MAP_FIXED : 0),
            -1, 0, errp);
    if (brk_aligned == (target_ulong) -1) {
        return -1;
    }

    prot = PROT_READ | PROT_WRITE;
    if (info->exec_stack) {
        prot |= PROT_EXEC;
    }

    guard = TARGET_PAGE_SIZE;
    stack_size = guest_stack_size;
    stack_limit = target_mmap(0, stack_size + guard, prot,
#ifndef __STRICT_ANSI__
            MAP_ANONYMOUS |
#endif
            MAP_PRIVATE, -1, 0, errp);
    if (stack_limit == -1) {
        get_errinfo(errp, "Cannot allocate stack space of process.");
        return -1;
    }

    if (target_mprotect(stack_limit, guard, PROT_NONE, errp) < 0) {
        return -1;
    }
    stack_limit += guard;
    info->stack_limit = stack_limit;
    bprm->p = stack_limit + stack_size - sizeof(void *); /* stack top */

    if (create_elf_tables(bprm, errp) < 0) {
        return -1;
    }
    info->start_stack = bprm->p;

    free(phdr);
    close(bprm->execfd);
    return 0;
}

/*
 * count() counts the number of strings in array ARGV.
 */
static int count(void *ptrs)
{
    int i = 0;
    void **p = ptrs;

    for (; *p != NULL; p++) {
        i++;
    }
    return i;
}

/*
 * Fill the binprm structure.
 * Read the first BINPRM_BUF_SIZE bytes
 */
static int prepare_binprm(struct linux_binprm *bprm, errinfo_t *errp)
{
    int n;

    memset(bprm->buf, 0, BINPRM_BUF_SIZE);

    n = readn(bprm->execfd, bprm->buf, BINPRM_BUF_SIZE);
    if (n < 0) {
        get_syserr(errp);
    }

    return n;
}

#ifdef CONFIG_TARGET_RISCV
static inline void init_thread(struct target_pt_regs *regs,
                               struct image_info *info)
{
    regs->sepc = info->entry;
    regs->sp = info->start_stack;
}
#endif

int elf_load(int fd, char *filename, char **argv, char **envp,
             struct linux_binprm *bprm, struct image_info *info,
             struct target_pt_regs *regs, errinfo_t *errp)
{
    bprm->execfd = fd;
    bprm->filename = filename;

    bprm->argc = count(argv);
    bprm->argv = argv;
    bprm->envc = count(envp);
    bprm->envp = envp;

    if (prepare_binprm(bprm, errp) < 0) {
        return -1;
    }

    if (load_elf_binary(bprm, info, errp) < 0) {
        return -1;
    }

    init_thread(regs, info);

    return 0;
}

