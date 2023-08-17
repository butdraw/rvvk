#ifndef LINUX_USER_RISCV_SYSCALL_DEFS_H
#define LINUX_USER_RISCV_SYSCALL_DEFS_H

#define TARGET_STAT_HAVE_NSEC
struct target_stat {
    abi_ulong st_dev;
    abi_ulong st_ino;
    abi_uint st_mode;
    abi_uint st_nlink;
    abi_uint st_uid;
    abi_uint st_gid;
    abi_ulong st_rdev;
    abi_ulong __pad1;
    abi_long st_size;
    abi_int st_blksize;
    abi_int __pad2;
    abi_long st_blocks;
    abi_long target_st_atime;
    abi_ulong target_st_atime_nsec;
    abi_long target_st_mtime;
    abi_ulong target_st_mtime_nsec;
    abi_long target_st_ctime;
    abi_ulong target_st_ctime_nsec;
    abi_uint __unused4;
    abi_uint __unused5;
};
#endif
