#include "include/rvvk.h"
#include "include/arch/riscv/syscall64_nr.h"
#include "include/arch/riscv/syscall_defs.h"

#include <fcntl.h>

static target_ulong target_brk, initial_target_brk;

void target_set_brk(target_ulong new_brk)
{
    target_brk = TARGET_PAGE_ALIGN(new_brk);
    initial_target_brk = target_brk;
}

target_ulong do_brk(target_ulong brk_val)
{
    target_brk = brk_val < initial_target_brk ? initial_target_brk : brk_val;
    return target_brk;
}

target_ulong do_syscall(state_t *env, int nr, target_ulong arg1, target_ulong arg2,
                        target_ulong arg3, target_ulong arg4, target_ulong arg5,
                        target_ulong arg6, target_ulong arg7, target_ulong arg8)
{
    logoutf("arg1: %016lx, arg2: %016lx, arg3: %016lx\n"
            "arg4: %016lx, arg5: %016lx, arg6: %016lx\n",
            arg1, arg2, arg3, arg4, arg5, arg6);
    int ret;
    struct stat st;
    switch (nr) {
    case TARGET_NR_faccessat:
#ifdef __STRICT_ANSI__
        return access(g2h(arg2), arg3);
#else
        return faccessat(arg1, g2h(arg2), arg3, 0);
#endif
    case TARGET_NR_fstat:
        ret = fstat(arg1, &st);
        {
            struct target_stat target_st, *s = &target_st;
            memset(s, 0, sizeof(*s));
            s->st_dev = st.st_dev;
            s->st_ino = st.st_ino;
            s->st_mode = st.st_mode;
            s->st_nlink = st.st_nlink;
            s->st_uid = st.st_uid;
            s->st_gid = st.st_gid;
            s->st_rdev = st.st_rdev;
            s->st_size = st.st_size;
            s->st_blksize = st.st_blksize;
            s->st_blocks = st.st_blocks;
            s->target_st_atime = st.st_atime;
            s->target_st_mtime = st.st_mtime;
            s->target_st_ctime = st.st_ctime;
#if defined(HAVE_STRUCT_STAT_ST_ATIM) && defined(TARGET_STAT_HAVE_NSEC)
#ifdef __STRICT_ANSI__
            s->target_st_atime_nsec = st.st_atimensec;
            s->target_st_mtime_nsec = st.st_mtimensec;
            s->target_st_ctime_nsec = st.st_ctimensec;
#else
            s->target_st_atime_nsec = st.st_atim.tv_nsec;
            s->target_st_mtime_nsec = st.st_mtim.tv_nsec;
            s->target_st_ctime_nsec = st.st_ctim.tv_nsec;
#endif
#endif
            target_memory_copy(arg2, s, sizeof(*s), NULL);
        }
        return ret;
    case TARGET_NR_write:
        return write(arg1, g2h(arg2), arg3);
    case TARGET_NR_close:
        // return close(arg1);
        return 0;
    case TARGET_NR_exit:
        _exit(arg1);
        return 0;
    case TARGET_NR_brk:
        return do_brk(arg1);
    case TARGET_NR_mmap:
        return target_mmap(arg1, arg2, arg3, arg4, arg5, arg6, NULL);
    default:
        logoutf("invalid ecall number!");
        exit(1);
    }
    return 0;
}
