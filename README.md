# rvvk
a Lightweight RISC-V Linux Visual Kernel

## What's working
- I/M/A Standard ISAs
- some simple syscalls (faccessat/fstat/write/close/exit/brk)
- Loading ELF that compiled statically used riscv-unknown-elf-gcc
- Translation block caching and chainning technology

## Usage
```
user@user:~ ./rvvk --help
Usage:
        rvvk [options] prog [args]
Options:
        -h, --help      display this message.
        -B, --address   set guest base address.
        -s, --stack     set guest stack size.
        -d, --debug     display debug logs.
```
