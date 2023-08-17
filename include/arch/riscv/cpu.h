#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#ifdef CONFIG_TARGET_RISCV
# define TARGET_PAGE_SIZE 4096
# define TARGET_ELF_EXEC_PAGESIZE 4096
#endif

#ifdef CONFIG_TARGET_RISCV64
typedef int32_t target_int;
typedef uint32_t target_uint;
typedef int64_t target_long;
typedef uint64_t target_ulong;
typedef uint32_t target_hulong;
typedef uint64_t target_uintptr;
#else
typedef int32_t target_int;
typedef uint32_t target_uint;
typedef int32_t target_long;
typedef uint32_t target_ulong;
typedef uint16_t target_hulong;
typedef uint32_t target_uintptr;
#endif /* CONFIG_TARGET_RISCV64 */

#define TARGET_LONG_BITS (sizeof(target_ulong) << 3)

/* riscv/include/asm/ptrace.h */
struct target_pt_regs {
    target_ulong sepc;
    target_ulong epc;
    target_ulong ra;
    target_ulong sp;
    target_ulong gp;
    target_ulong tp;
    target_ulong t0;
    target_ulong t1;
    target_ulong t2;
    target_ulong s0;
    target_ulong s1;
    target_ulong a0;
    target_ulong a1;
    target_ulong a2;
    target_ulong a3;
    target_ulong a4;
    target_ulong a5;
    target_ulong a6;
    target_ulong a7;
    target_ulong s2;
    target_ulong s3;
    target_ulong s4;
    target_ulong s5;
    target_ulong s6;
    target_ulong s7;
    target_ulong s8;
    target_ulong s9;
    target_ulong s10;
    target_ulong s11;
    target_ulong t3;
    target_ulong t4;
    target_ulong t5;
    target_ulong t6;
    /* Supervisor/Machine CSRs */
    target_ulong status;
    target_ulong badaddr;
    target_ulong cause;
    /* a0 value before the syscall */
    target_ulong orig_a0;
};

#define xRA 1   /* return address (aka link register) */
#define xSP 2   /* stack pointer */
#define xGP 3   /* global pointer */
#define xTP 4   /* thread pointer */

#define xA0 10  /* gpr[10-17] are syscall arguments */
#define xA1 11
#define xA2 12
#define xA3 13
#define xA4 14
#define xA5 15
#define xA6 16
#define xA7 17  /* syscall number for RVI ABI */
#define xT0 5   /* syscall number for RVE ABI */

/* User Floating-Point CSRs */
#define CSR_FFLAGS          0x001
#define CSR_FRM             0x002
#define CSR_FCSR            0x003

/* User Vector CSRs */
#define CSR_VSTART          0x008
#define CSR_VXSAT           0x009
#define CSR_VXRM            0x00a
#define CSR_VCSR            0x00f
#define CSR_VL              0xc20
#define CSR_VTYPE           0xc21
#define CSR_VLENB           0xc22

/* Zcmt Extension */
#define CSR_JVT             0x017

typedef struct {
    target_ulong gpr[32];
    target_ulong pc;
    /* User Floating-Point CSRs */
    target_ulong fflags;
    target_ulong frm;
    target_ulong fcsr;
    /* Vector CSRs */
    target_ulong vstart;
    target_ulong vxsat;
    target_ulong vxrm;
    target_ulong vcsr;
    target_ulong vl;
    target_ulong vtype;
    target_ulong vlenb;
    /* Zcmt Extension */
    target_ulong jvt;
    int exception;
} state_t;

enum {
    RISCV_EXCP_NONE = -1, /* sentinel value */
    RISCV_EXCP_INST_ADDR_MIS = 0x0,
    RISCV_EXCP_INST_ACCESS_FAULT = 0x1,
    RISCV_EXCP_ILLEGAL_INST = 0x2,
    RISCV_EXCP_BREAKPOINT = 0x3,
    RISCV_EXCP_LOAD_ADDR_MIS = 0x4,
    RISCV_EXCP_LOAD_ACCESS_FAULT = 0x5,
    RISCV_EXCP_STORE_AMO_ADDR_MIS = 0x6,
    RISCV_EXCP_STORE_AMO_ACCESS_FAULT = 0x7,
    RISCV_EXCP_U_ECALL = 0x8,
    RISCV_EXCP_S_ECALL = 0x9,
    RISCV_EXCP_VS_ECALL = 0xa,
    RISCV_EXCP_M_ECALL = 0xb,
    RISCV_EXCP_INST_PAGE_FAULT = 0xc, /* since: priv-1.10.0 */
    RISCV_EXCP_LOAD_PAGE_FAULT = 0xd, /* since: priv-1.10.0 */
    RISCV_EXCP_STORE_PAGE_FAULT = 0xf, /* since: priv-1.10.0 */
    RISCV_EXCP_SEMIHOST = 0x10,
    RISCV_EXCP_INST_GUEST_PAGE_FAULT = 0x14,
    RISCV_EXCP_LOAD_GUEST_ACCESS_FAULT = 0x15,
    RISCV_EXCP_VIRT_INSTRUCTION_FAULT = 0x16,
    RISCV_EXCP_STORE_GUEST_AMO_ACCESS_FAULT = 0x17,
};

typedef struct insn_t insn_t;
typedef void (*insn_handler_t) (state_t *, insn_t *);

struct insn_t {
    int rs1;
    int rs2;
    int rd;
    union {
        int imm;
        int csr;
    };
    target_ulong pc;
    insn_handler_t handler;
};

typedef struct trans_block_t trans_block_t;
struct trans_block_t {
    target_ulong pc;
    insn_t *insn;
    int count;
    struct {
        target_ulong pc;
        trans_block_t *blk;
    } self, jmp_next[2], *last_jmp_next;
    trans_block_t *hash_next; /* avoid collision */
};

#define TRANS_MAX 512
extern void *insn_buffer_ptr;


void do_empty(state_t *env, insn_t *insn);
void do_illegal(state_t *env, insn_t *insn);

void do_ecall(state_t *env, insn_t *insn);

void do_auipc(state_t *env, insn_t *insn);
void do_lui(state_t *env, insn_t *insn);
void do_jalr(state_t *env, insn_t *insn);
void do_jal(state_t *env, insn_t *insn);
void do_c_jal(state_t *env, insn_t *insn);
void do_c_jalr(state_t *env, insn_t *insn);
void do_add(state_t *env, insn_t *insn);
void do_sub(state_t *env, insn_t *insn);
void do_sll(state_t *env, insn_t *insn);
void do_slt(state_t *env, insn_t *insn);
void do_sltu(state_t *env, insn_t *insn);
void do_xor(state_t *env, insn_t *insn);
void do_srl(state_t *env, insn_t *insn);
void do_sra(state_t *env, insn_t *insn);
void do_or(state_t *env, insn_t *insn);
void do_and(state_t *env, insn_t *insn);

void do_beq(state_t *env, insn_t *insn);
void do_bne(state_t *env, insn_t *insn);
void do_blt(state_t *env, insn_t *insn);
void do_bge(state_t *env, insn_t *insn);
void do_bltu(state_t *env, insn_t *insn);
void do_bgeu(state_t *env, insn_t *insn);
void do_c_beq(state_t *env, insn_t *insn);
void do_c_bne(state_t *env, insn_t *insn);

void do_lb(state_t *env, insn_t *insn);
void do_lh(state_t *env, insn_t *insn);
void do_lw(state_t *env, insn_t *insn);
void do_lbu(state_t *env, insn_t *insn);
void do_lhu(state_t *env, insn_t *insn);

void do_sb(state_t *env, insn_t *insn);
void do_sh(state_t *env, insn_t *insn);
void do_sw(state_t *env, insn_t *insn);

void do_addi(state_t *env, insn_t *insn);
void do_slti(state_t *env, insn_t *insn);
void do_sltiu(state_t *env, insn_t *insn);
void do_xori(state_t *env, insn_t *insn);
void do_ori(state_t *env, insn_t *insn);
void do_andi(state_t *env, insn_t *insn);

void do_slli(state_t *env, insn_t *insn);
void do_srli(state_t *env, insn_t *insn);
void do_srai(state_t *env, insn_t *insn);

void do_lwu(state_t *env, insn_t *insn);
void do_ld(state_t *env, insn_t *insn);
void do_sd(state_t *env, insn_t *insn);
void do_addiw(state_t *env, insn_t *insn);
void do_slliw(state_t *env, insn_t *insn);
void do_srliw(state_t *env, insn_t *insn);
void do_sraiw(state_t *env, insn_t *insn);
void do_addw(state_t *env, insn_t *insn);
void do_subw(state_t *env, insn_t *insn);
void do_sllw(state_t *env, insn_t *insn);
void do_srlw(state_t *env, insn_t *insn);
void do_sraw(state_t *env, insn_t *insn);

void do_mul(state_t *env, insn_t *insn);
void do_mulh(state_t *env, insn_t *insn);
void do_mulhsu(state_t *env, insn_t *insn);
void do_mulhu(state_t *env, insn_t *insn);
void do_div(state_t *env, insn_t *insn);
void do_divu(state_t *env, insn_t *insn);
void do_rem(state_t *env, insn_t *insn);
void do_remu(state_t *env, insn_t *insn);

void do_mulw(state_t *env, insn_t *insn);
void do_divw(state_t *env, insn_t *insn);
void do_divuw(state_t *env, insn_t *insn);
void do_remw(state_t *env, insn_t *insn);
void do_remuw(state_t *env, insn_t *insn);

void do_csrrw(state_t *env, insn_t *insn);
void do_csrrs(state_t *env, insn_t *insn);
void do_csrrc(state_t *env, insn_t *insn);
void do_csrrwi(state_t *env, insn_t *insn);
void do_csrrsi(state_t *env, insn_t *insn);
void do_csrrci(state_t *env, insn_t *insn);

void do_lr_w(state_t *env, insn_t *insn);
void do_sc_w(state_t *env, insn_t *insn);
void do_amoadd_w(state_t *env, insn_t *insn);
void do_amoswap_w(state_t *env, insn_t *insn);
void do_amoxor_w(state_t *env, insn_t *insn);
void do_amoor_w(state_t *env, insn_t *insn);
void do_amoand_w(state_t *env, insn_t *insn);
void do_amomin_w(state_t *env, insn_t *insn);
void do_amomax_w(state_t *env, insn_t *insn);
void do_amominu_w(state_t *env, insn_t *insn);
void do_amomaxu_w(state_t *env, insn_t *insn);

void do_lr_d(state_t *env, insn_t *insn);
void do_sc_d(state_t *env, insn_t *insn);
void do_amoadd_d(state_t *env, insn_t *insn);
void do_amoswap_d(state_t *env, insn_t *insn);
void do_amoxor_d(state_t *env, insn_t *insn);
void do_amoor_d(state_t *env, insn_t *insn);
void do_amoand_d(state_t *env, insn_t *insn);
void do_amomin_d(state_t *env, insn_t *insn);
void do_amomax_d(state_t *env, insn_t *insn);
void do_amominu_d(state_t *env, insn_t *insn);
void do_amomaxu_d(state_t *env, insn_t *insn);

trans_block_t *translate(trans_block_t *block);

#endif
