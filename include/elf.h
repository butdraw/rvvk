#ifndef ELF_H
#define ELF_H

#include <stdint.h>

#include "config.h"

/* Type for a 16-bit quantity.  */
typedef uint16_t Elf32_Half;
typedef uint16_t Elf64_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;

/* Types for signed and unsigned 64-bit quantities.  */
typedef uint64_t Elf32_Xword;
typedef int64_t  Elf32_Sxword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

/* Type of addresses.  */
typedef uint32_t Elf32_Addr;
typedef uint64_t Elf64_Addr;

/* Type of file offsets.  */
typedef uint32_t Elf32_Off;
typedef uint64_t Elf64_Off;

/* Type for section indices, which are 16-bit quantities.  */
typedef uint16_t Elf32_Section;
typedef uint16_t Elf64_Section;

/* Type for version symbol information.  */
typedef Elf32_Half Elf32_Versym;
typedef Elf64_Half Elf64_Versym;

/* The ELF file header.  This appears at the start of every ELF file.  */

#define EI_NIDENT (16)

typedef struct
{
  unsigned char e_ident[EI_NIDENT];     /* Magic number and other info */
  Elf32_Half    e_type;                 /* Object file type */
  Elf32_Half    e_machine;              /* Architecture */
  Elf32_Word    e_version;              /* Object file version */
  Elf32_Addr    e_entry;                /* Entry point virtual address */
  Elf32_Off     e_phoff;                /* Program header table file offset */
  Elf32_Off     e_shoff;                /* Section header table file offset */
  Elf32_Word    e_flags;                /* Processor-specific flags */
  Elf32_Half    e_ehsize;               /* ELF header size in bytes */
  Elf32_Half    e_phentsize;            /* Program header table entry size */
  Elf32_Half    e_phnum;                /* Program header table entry count */
  Elf32_Half    e_shentsize;            /* Section header table entry size */
  Elf32_Half    e_shnum;                /* Section header table entry count */
  Elf32_Half    e_shstrndx;             /* Section header string table index */
} Elf32_Ehdr;

typedef struct
{
  unsigned char e_ident[EI_NIDENT];     /* Magic number and other info */
  Elf64_Half    e_type;                 /* Object file type */
  Elf64_Half    e_machine;              /* Architecture */
  Elf64_Word    e_version;              /* Object file version */
  Elf64_Addr    e_entry;                /* Entry point virtual address */
  Elf64_Off     e_phoff;                /* Program header table file offset */
  Elf64_Off     e_shoff;                /* Section header table file offset */
  Elf64_Word    e_flags;                /* Processor-specific flags */
  Elf64_Half    e_ehsize;               /* ELF header size in bytes */
  Elf64_Half    e_phentsize;            /* Program header table entry size */
  Elf64_Half    e_phnum;                /* Program header table entry count */
  Elf64_Half    e_shentsize;            /* Section header table entry size */
  Elf64_Half    e_shnum;                /* Section header table entry count */
  Elf64_Half    e_shstrndx;             /* Section header string table index */
} Elf64_Ehdr;

/* Fields in the e_ident array.  The EI_* macros are indices into the
   array.  The macros under each EI_* macro are the values the byte
   may have.  */

#define EI_MAG0         0               /* File identification byte 0 index */
#define ELFMAG0         0x7f            /* Magic number byte 0 */

#define EI_MAG1         1               /* File identification byte 1 index */
#define ELFMAG1         'E'             /* Magic number byte 1 */

#define EI_MAG2         2               /* File identification byte 2 index */
#define ELFMAG2         'L'             /* Magic number byte 2 */

#define EI_MAG3         3               /* File identification byte 3 index */
#define ELFMAG3         'F'             /* Magic number byte 3 */

/* Conglomeration of the identification bytes, for easy testing as a word.  */
#define ELFMAG          "\177ELF"
#define SELFMAG         4

#define EI_CLASS        4               /* File class byte index */
#define ELFCLASSNONE    0               /* Invalid class */
#define ELFCLASS32      1               /* 32-bit objects */
#define ELFCLASS64      2               /* 64-bit objects */
#define ELFCLASSNUM     3

#define EI_DATA         5               /* Data encoding byte index */
#define ELFDATANONE     0               /* Invalid data encoding */
#define ELFDATA2LSB     1               /* 2's complement, little endian */
#define ELFDATA2MSB     2               /* 2's complement, big endian */
#define ELFDATANUM      3

#define EI_VERSION      6               /* File version byte index */
                                        /* Value must be EV_CURRENT */

#define EI_OSABI        7               /* OS ABI identification */
#define ELFOSABI_NONE           0       /* UNIX System V ABI */
#define ELFOSABI_SYSV           0       /* Alias.  */
#define ELFOSABI_HPUX           1       /* HP-UX */
#define ELFOSABI_NETBSD         2       /* NetBSD.  */
#define ELFOSABI_GNU            3       /* Object uses GNU ELF extensions.  */
#define ELFOSABI_LINUX          ELFOSABI_GNU /* Compatibility alias.  */
#define ELFOSABI_SOLARIS        6       /* Sun Solaris.  */
#define ELFOSABI_AIX            7       /* IBM AIX.  */
#define ELFOSABI_IRIX           8       /* SGI Irix.  */
#define ELFOSABI_FREEBSD        9       /* FreeBSD.  */
#define ELFOSABI_TRU64          10      /* Compaq TRU64 UNIX.  */
#define ELFOSABI_MODESTO        11      /* Novell Modesto.  */
#define ELFOSABI_OPENBSD        12      /* OpenBSD.  */
#define ELFOSABI_ARM_AEABI      64      /* ARM EABI */
#define ELFOSABI_ARM            97      /* ARM */
#define ELFOSABI_STANDALONE     255     /* Standalone (embedded) application */

#define EI_ABIVERSION   8               /* ABI version */

#define EI_PAD          9               /* Byte index of padding bytes */

/* Legal values for e_version (version).  */

#define EV_NONE         0               /* Invalid ELF version */
#define EV_CURRENT      1               /* Current version */
#define EV_NUM          2

/* Legal values for e_type (object file type).  */

#define ET_NONE         0               /* No file type */
#define ET_REL          1               /* Relocatable file */
#define ET_EXEC         2               /* Executable file */
#define ET_DYN          3               /* Shared object file */
#define ET_CORE         4               /* Core file */
#define ET_NUM          5               /* Number of defined types */
#define ET_LOOS         0xfe00          /* OS-specific range start */
#define ET_HIOS         0xfeff          /* OS-specific range end */
#define ET_LOPROC       0xff00          /* Processor-specific range start */
#define ET_HIPROC       0xffff          /* Processor-specific range end */

#define EM_RISCV        243     /* RISC-V */

#ifdef CONFIG_TARGET_RISCV
#define EM_ARCH EM_RISCV
#endif /* CONFIG_TARGET_RISCV */

#ifdef CONFIG_TARGET_RISCV64
typedef Elf64_Ehdr elf_ehdr_t;
#define ELF_CLASS ELFCLASS64
#else
typedef Elf32_Ehdr elf_ehdr_t;
#define ELF_CLASS ELFCLASS32
#endif /* CONFIG_TARGET_RISCV64 */

#ifdef CONFIG_TARGET_LITTLE_ENDIAN
#define ELF_DATA ELFDATA2LSB
#else
#define ELF_DATA ELFDATA2MSB
#endif /* CONFIG_TARGET_LITTLE_ENDIAN */

/* Program segment header.  */

typedef struct
{
  Elf32_Word    p_type;                 /* Segment type */
  Elf32_Off     p_offset;               /* Segment file offset */
  Elf32_Addr    p_vaddr;                /* Segment virtual address */
  Elf32_Addr    p_paddr;                /* Segment physical address */
  Elf32_Word    p_filesz;               /* Segment size in file */
  Elf32_Word    p_memsz;                /* Segment size in memory */
  Elf32_Word    p_flags;                /* Segment flags */
  Elf32_Word    p_align;                /* Segment alignment */
} Elf32_Phdr;

typedef struct
{
  Elf64_Word    p_type;                 /* Segment type */
  Elf64_Word    p_flags;                /* Segment flags */
  Elf64_Off     p_offset;               /* Segment file offset */
  Elf64_Addr    p_vaddr;                /* Segment virtual address */
  Elf64_Addr    p_paddr;                /* Segment physical address */
  Elf64_Xword   p_filesz;               /* Segment size in file */
  Elf64_Xword   p_memsz;                /* Segment size in memory */
  Elf64_Xword   p_align;                /* Segment alignment */
} Elf64_Phdr;

/* Special value for e_phnum.  This indicates that the real number of
   program headers is too large to fit into e_phnum.  Instead the real
   value is in the field sh_info of section 0.  */

#define PN_XNUM         0xffff

/* Legal values for p_type (segment type).  */

#define PT_NULL         0               /* Program header table entry unused */
#define PT_LOAD         1               /* Loadable program segment */
#define PT_DYNAMIC      2               /* Dynamic linking information */
#define PT_INTERP       3               /* Program interpreter */
#define PT_NOTE         4               /* Auxiliary information */
#define PT_SHLIB        5               /* Reserved */
#define PT_PHDR         6               /* Entry for header table itself */
#define PT_TLS          7               /* Thread-local storage segment */
#define PT_NUM          8               /* Number of defined types */
#define PT_LOOS         0x60000000      /* Start of OS-specific */
#define PT_GNU_EH_FRAME 0x6474e550      /* GCC .eh_frame_hdr segment */
#define PT_GNU_STACK    0x6474e551      /* Indicates stack executability */
#define PT_GNU_RELRO    0x6474e552      /* Read-only after relocation */
#define PT_GNU_PROPERTY 0x6474e553      /* GNU property */
#define PT_LOSUNW       0x6ffffffa
#define PT_SUNWBSS      0x6ffffffa      /* Sun Specific segment */
#define PT_SUNWSTACK    0x6ffffffb      /* Stack segment */
#define PT_HISUNW       0x6fffffff
#define PT_HIOS         0x6fffffff      /* End of OS-specific */
#define PT_LOPROC       0x70000000      /* Start of processor-specific */
#define PT_HIPROC       0x7fffffff      /* End of processor-specific */

/* Legal values for p_flags (segment flags).  */

#define PF_X            (1 << 0)        /* Segment is executable */
#define PF_W            (1 << 1)        /* Segment is writable */
#define PF_R            (1 << 2)        /* Segment is readable */
#define PF_MASKOS       0x0ff00000      /* OS-specific */
#define PF_MASKPROC     0xf0000000      /* Processor-specific */

typedef struct
{
  uint32_t a_type;		/* Entry type */
  union
    {
      uint32_t a_val;		/* Integer value */
      /* We use to have pointer elements added here.  We cannot do that,
	 though, since it does not work when using 32-bit definitions
	 on 64-bit platforms and vice versa.  */
    } a_un;
} Elf32_auxv_t;

typedef struct
{
  uint64_t a_type;		/* Entry type */
  union
    {
      uint64_t a_val;		/* Integer value */
      /* We use to have pointer elements added here.  We cannot do that,
	 though, since it does not work when using 32-bit definitions
	 on 64-bit platforms and vice versa.  */
    } a_un;
} Elf64_auxv_t;

#include <bits/auxv.h>

#ifdef CONFIG_TARGET_RISCV64
typedef Elf64_Phdr elf_phdr_t;
typedef Elf64_auxv_t elf_auxv_t;
#define ELF_CLASS ELFCLASS64
#else
typedef Elf32_Phdr elf_phdr_t;
typedef Elf32_auxv_t elf_auxv_t;
#define ELF_CLASS ELFCLASS32
#endif /* CONFIG_TARGET_RISCV64 */

#endif
