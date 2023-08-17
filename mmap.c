#include "include/rvvk.h"

#if CONFIG_HOST_LONG_BITS == 64 && CONFIG_TARGET_ABI_BITS == 64
# define TASK_UNMAPPED_BASE  0x4000000000
#else
# define TASK_UNMAPPED_BASE  0x40000000
#endif

static target_ulong mmap_next_start = TASK_UNMAPPED_BASE;

void *host_mmap(void *addr, size_t length, int prot,
                int flags, int fd, off_t offset, errinfo_t *errp)
{
    void *p;
#ifdef __STRICT_ANSI__
    int need_closefd = 0;
    if (fd < 0) {
        fd = open("/dev/zero", O_RDWR);
        if (fd == -1) {
            get_syserr(errp);
            return MAP_FAILED;
        }
        need_closefd = 1;
    }
#endif
    p = mmap(addr, length, prot, flags, fd, offset);
#ifdef __STRICT_ANSI__
    if (need_closefd) {
        close(fd);
    }
#endif
    if (p == MAP_FAILED) {
        get_syserr(errp);
        return MAP_FAILED;
    }

    return p;
}

target_ulong mmap_find_vma(target_ulong addr, size_t length, errinfo_t *errp)
{
    void *p;
    if (addr == 0) {
        addr = mmap_next_start;
    } else {
        addr &= HOST_PAGE_MASK;
    }
    length = HOST_PAGE_ALIGN(length);
    /*
     * Reserve needed memory area to avoid a race.
     * TODO: It's not easy to cache visual mapping region with MAP_FIXED flag.
     */
    p = host_mmap(g2h(addr), length, PROT_NONE,
#ifndef __STRICT_ANSI__
            MAP_NORESERVE | MAP_ANONYMOUS |
#endif
            MAP_PRIVATE | MAP_FIXED, -1, 0, errp);
    if (p == MAP_FAILED) {
        return -1;
    }
    if (h2g(p) == mmap_next_start && addr >= TASK_UNMAPPED_BASE) {
        mmap_next_start += length;
    }
    return addr;
}

target_ulong target_mmap(target_ulong addr, size_t length, int prot,
                         int flags, int fd, off_t offset, errinfo_t *errp)
{
    void *p;

    /* If the user is asking for the kernel to find a location.  */
    if (!(flags & MAP_FIXED)) {
        /* Find a correct location.  */
        addr = mmap_find_vma(addr, length, errp);
        if (addr == -1) {
            return -1;
        }
        flags |=
#ifndef __STRICT_ANSI__
            MAP_ANONYMOUS |
#endif
            MAP_FIXED;
    }
    p = host_mmap(g2h(addr), length, prot, flags, fd, offset, errp);
    if (p == MAP_FAILED) {
        get_syserr(errp);
        return -1;
    }

    return addr;
}

int target_mprotect(target_ulong addr, target_ulong len, int prot, errinfo_t *errp)
{
    int ret = mprotect(g2h(addr), len, prot);
    if (ret == -1) {
        get_syserr(errp);
        return -1;
    }
    return 0;
}

int target_memory_copy(target_ulong addr, void *src, size_t len, errinfo_t *errp)
{
    void *p = memcpy(g2h(addr), src, len);
    if (p != g2h(addr)) {
        get_syserr(errp);
        return -1;
    }
    return 0;
}

uint64_t cpu_ldud(target_ulong addr, target_long offset)
{
    return *(uint64_t *)g2h(addr + offset);
}

uint32_t cpu_lduw(target_ulong addr, target_long offset)
{
    return *(uint32_t *)g2h(addr + offset);
}

uint16_t cpu_lduh(target_ulong addr, target_long offset)
{
    return *(uint16_t *)g2h(addr + offset);
}

uint8_t cpu_ldub(target_ulong addr, target_long offset)
{
    return *(uint8_t *)g2h(addr + offset);
}


void cpu_stud(target_ulong addr, target_long offset, uint64_t value)
{
    *(uint64_t *)g2h(addr + offset) = value;
}

void cpu_stuw(target_ulong addr, target_long offset, uint32_t value)
{
    *(uint32_t *)g2h(addr + offset) = value;
}

void cpu_stuh(target_ulong addr, target_long offset, uint16_t value)
{
    *(uint16_t *)g2h(addr + offset) = value;
}

void cpu_stub(target_ulong addr, target_long offset, uint8_t value)
{
    *(uint8_t *)g2h(addr + offset) = value;
}
