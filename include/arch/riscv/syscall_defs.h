#ifndef LINUX_USER_RISCV_SYSCALL_DEFS_H
#define LINUX_USER_RISCV_SYSCALL_DEFS_H

#include "cpu.h"

#define TARGET_STAT_HAVE_NSEC
struct target_stat {
    target_ulong st_dev;
    target_ulong st_ino;
    target_uint st_mode;
    target_uint st_nlink;
    target_uint st_uid;
    target_uint st_gid;
    target_ulong st_rdev;
    target_ulong __pad1;
    target_long st_size;
    target_int st_blksize;
    target_int __pad2;
    target_long st_blocks;
    target_long target_st_atime;
#if defined(HAVE_STRUCT_STAT_ST_ATIM) && defined(TARGET_STAT_HAVE_NSEC)
    target_ulong target_st_atime_nsec;
#endif
    target_long target_st_mtime;
#if defined(HAVE_STRUCT_STAT_ST_ATIM) && defined(TARGET_STAT_HAVE_NSEC)
    target_ulong target_st_mtime_nsec;
#endif
    target_long target_st_ctime;
#if defined(HAVE_STRUCT_STAT_ST_ATIM) && defined(TARGET_STAT_HAVE_NSEC)
    target_ulong target_st_ctime_nsec;
#endif
    target_uint __unused4;
    target_uint __unused5;
};
#endif
