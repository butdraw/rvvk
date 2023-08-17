#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_HOST_LONG_BITS 64

#define CONFIG_TARGET_RISCV64
#define CONFIG_TARGET_ABI_BITS 64
#define CONFIG_TARGET_LITTLE_ENDIAN

#define HAVE_STRUCT_STAT_ST_ATIM
#define TARGET_STAT_HAVE_NSEC

#if defined CONFIG_TARGET_RISCV64 || CONFIG_TARGET_RISCV32
#define CONFIG_TARGET_RISCV
#ifndef CONFIG_TARGET
# define CONFIG_TARGET
#else
# error "You have specified a architecture."
#endif
#endif


#ifndef CONFIG_TARGET
#error "Must configure a specified architecture."
#endif

#endif
