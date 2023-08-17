#ifndef RVVK_H
#define RVVK_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>

#include "config.h"

#ifdef CONFIG_TARGET_RISCV
#include "arch/riscv/cpu.h"
#endif

/* util */

#define KiB     (1ULL << 10)
#define MiB     (1ULL << 20)
#define GiB     (1ULL << 30)
#define TiB     (1ULL << 40)
#define PiB     (1ULL << 50)
#define EiB     (1ULL << 60)

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/* Throw a rational prompt for unreachable code.  */
#define reached (0)
#define assert_not_reached() assert(reached)

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#ifndef ROUND_DOWN
#define ROUND_DOWN(n, d) ((n) & -(0 ? (n) : (d)))
#endif
#ifndef ROUND_UP
#define ROUND_UP(n, d) ROUND_DOWN((n) + (d) - 1, (d))
#endif

typedef struct {
    const char *file;
    unsigned int line;
    const char *func;
    const char *info;
} errinfo_t;

#define get_errinfo(errp, reason) \
do {                              \
    if (errp) {                   \
        errp->file = __FILE__;    \
        errp->line = __LINE__;    \
        errp->func = __func__;    \
        errp->info = reason;      \
    }                             \
} while (0)

#define get_syserr(errp) \
    get_errinfo(errp, strerror(errno))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifndef min
#define min(x, y) (x) < (y) ? (x) : (y)
#endif

#ifndef max
#define max(x, y) (x) > (y) ? (x) : (y)
#endif

#define atomic_xchg(ptr, i) ({                       \
    __atomic_exchange_n(ptr, (i), __ATOMIC_SEQ_CST); \
})
#define atomic_cmpxchg(ptr, old, new)    ({                        \
    typeof_strip_qual(*ptr) _old = (old);                          \
    (void)__atomic_compare_exchange_n(ptr, &_old, new, false,      \
                              __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); \
    _old;                                                          \
})
#define atomic_add(ptr, n) \
    (__atomic_fetch_add(ptr, n, __ATOMIC_SEQ_CST))
#define atomic_sub(ptr, n) \
    (__atomic_fetch_sub(ptr, n, __ATOMIC_SEQ_CST))
#define atomic_and(ptr, n) \
    (__atomic_fetch_and(ptr, n, __ATOMIC_SEQ_CST))
#define atomic_or(ptr, n) \
    (__atomic_fetch_or(ptr, n, __ATOMIC_SEQ_CST))
#define atomic_xor(ptr, n) \
    (__atomic_fetch_xor(ptr, n, __ATOMIC_SEQ_CST))

typedef int (*log_handler)(const char *fmt, ...);
extern log_handler logoutf;
char *strdupf(const char *fmt, ...);
int parse_options(char **opts, char **prog_path, char ***prog_argv,
                  char ***prog_envp, errinfo_t *errp);
char *get_real_path(const char *path, char *real_path, errinfo_t *errp);

ssize_t readn(int fd, char *buf, size_t count);
#ifdef __STRICT_ANSI__
ssize_t pread(int fd, void *buf, size_t count, off_t offset);
#endif

/* bitops */

static inline uint32_t extract32(uint32_t value, int start, int length)
{
    assert(start >= 0 && length > 0 && length <= 32 - start);
    return (value >> start) & (~0U >> (32 - length));
}

static inline int32_t sextract32(uint32_t value, int start, int length)
{
    assert(start >= 0 && length > 0 && length <= 32 - start);
    /* Note that this implementation relies on right shift of signed
     * integers being an arithmetic shift.
     */
    return ((int32_t)(value << (32 - length - start))) >> (32 - length);
}

static inline uint32_t deposit32(uint32_t value, int start, int length,
                                 uint32_t fieldval)
{
    uint32_t mask;
    assert(start >= 0 && length > 0 && length <= 32 - start);
    mask = (~0U >> (32 - length)) << start;
    return (value & ~mask) | ((fieldval << start) & mask);
}

/* main */

#include <sys/user.h>
#define HOST_PAGE_SIZE (PAGE_SIZE)
#define HOST_PAGE_MASK (-PAGE_SIZE)
#define HOST_PAGE_ALIGN(addr) (ROUND_UP(addr, HOST_PAGE_SIZE))

/* linux v6.4.8: fs/binfmt_elf.c */
#if TARGET_ELF_EXEC_PAGESIZE > TARGET_PAGE_SIZE
# define TARGET_ELF_MIN_ALIGN TARGET_ELF_EXEC_PAGESIZE
#else
# define TARGET_ELF_MIN_ALIGN TARGET_PAGE_SIZE
#endif /* TARGET_ELF_EXEC_PAGESIZE > TARGET_PAGE_SIZE */

#define TARGET_ELF_PAGESTART(_v)  ((_v) & ~(int)(TARGET_ELF_MIN_ALIGN - 1))
#define TARGET_ELF_PAGEOFFSET(_v) ((_v) & (TARGET_ELF_MIN_ALIGN - 1))
#define TARGET_ELF_PAGEALIGN(_v) \
   (((_v) + TARGET_ELF_MIN_ALIGN - 1) & ~(TARGET_ELF_MIN_ALIGN - 1))
#define TARGET_ELF_PAGELENGTH(_v) ROUND_UP((_v), TARGET_ELF_EXEC_PAGESIZE)

#define TARGET_PAGE_ALIGN(addr) (ROUND_UP(addr, TARGET_PAGE_SIZE))

/*
 * This structure is used to hold the arguments that are
 * used when loading binaries.
 */
#define BINPRM_BUF_SIZE 256
struct linux_binprm {
    char buf[BINPRM_BUF_SIZE] __attribute__((aligned));
    target_ulong p; /* current top of mem */
    int execfd;
    // int e_uid, e_gid;
    int argc, envc;
    char **argv;
    char **envp;
    char *filename;        /* Name of binary */
    // int (*core_dump)(int, const CPUArchState *); /* coredump routine */
};

struct image_info {

    target_ulong load_bias;
    target_ulong load_addr;
    target_ulong start_code;
    target_ulong end_code;
    target_ulong start_data;
    target_ulong end_data;

    target_ulong start_brk;
    target_ulong brk;
    target_ulong reserve_brk;

    target_ulong start_mmap;
    target_ulong start_stack;
    target_ulong stack_limit;
    target_ulong entry;
    target_ulong code_offset;
    target_ulong data_offset;
    target_ulong saved_auxv;
    target_ulong auxv_len;
    target_ulong argc;
    target_ulong argv;
    target_ulong envc;
    target_ulong envp;
    target_ulong file_string;
    uint32_t     elf_flags;
    int          personality;
    target_ulong alignment;
    int          exec_stack;

    // /* Generic semihosting knows about these pointers. */
    // abi_ulong       arg_strings;   /* strings for argv */
    // abi_ulong       env_strings;   /* strings for envp; ends arg_strings */

    // /* The fields below are used in FDPIC mode.  */
    // abi_ulong       loadmap_addr;
    uint16_t        nsegs;
    // void            *loadsegs;
    // abi_ulong       pt_dynamic_addr;
    // abi_ulong       interpreter_loadmap_addr;
    // abi_ulong       interpreter_pt_dynamic_addr;
    // struct image_info *other_info;

    // /* For target-specific processing of NT_GNU_PROPERTY_TYPE_0. */
    // uint32_t        note_flags;
};

int elf_load(int fd, char *filename, char **argv, char **envp,
             struct linux_binprm *bprm, struct image_info *info,
             struct target_pt_regs *regs, errinfo_t *errp);
void target_set_brk(target_ulong new_brk);

typedef struct {
    target_ulong stack_base;
    target_ulong heap_base;
    target_ulong heap_limit;
} task_t;

/* mmap */

extern unsigned long guest_base;
extern unsigned long guest_stack_size;
static inline void *g2h(target_ulong x)
{
    return (void *)(x + guest_base);
}
static inline target_ulong h2g(void *x)
{
    return (target_ulong)((uintptr_t)x - guest_base);
}

void *host_mmap(void *addr, size_t length, int prot,
                int flags, int fd, off_t offset, errinfo_t *errp);
target_ulong target_mmap(target_ulong target_addr, size_t length, int prot,
                         int flags, int fd, off_t offset, errinfo_t *errp);
int target_mprotect(target_ulong addr, target_ulong len, int prot, errinfo_t *errp);
int target_memory_copy(target_ulong addr, void *src, size_t len, errinfo_t *errp);

uint64_t cpu_ldud(target_ulong addr, target_long offset);
uint32_t cpu_lduw(target_ulong addr, target_long offset);
uint16_t cpu_lduh(target_ulong addr, target_long offset);
uint8_t cpu_ldub(target_ulong addr, target_long offset);
void cpu_stud(target_ulong addr, target_long offset, uint64_t value);
void cpu_stuw(target_ulong addr, target_long offset, uint32_t value);
void cpu_stuh(target_ulong addr, target_long offset, uint16_t value);
void cpu_stub(target_ulong addr, target_long offset, uint8_t value);

/* arch */

void target_cpu_copy_regs(state_t *env, task_t *ts,
                          struct image_info *info,
                          struct target_pt_regs *regs);

void cpu_loop(state_t *env);

/* syscall */

target_ulong do_syscall(state_t *env, int nr, target_ulong arg1, target_ulong arg2,
                        target_ulong arg3, target_ulong arg4, target_ulong arg5,
                        target_ulong arg6, target_ulong arg7, target_ulong arg8);

#endif
