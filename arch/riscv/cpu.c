#include "../../include/rvvk.h"

void target_cpu_copy_regs(state_t *env, task_t *ts,
                          struct image_info *info,
                          struct target_pt_regs *regs)
{
    env->pc = regs->sepc;
    env->gpr[xSP] = regs->sp;

    ts->stack_base = info->start_stack;
    ts->heap_base = info->brk;
    /* This will be filled in on the first SYS_HEAPINFO call.  */
    ts->heap_limit = 0;
}

void do_empty(state_t *env, insn_t *insn)
{}

void do_illegal(state_t *env, insn_t *insn)
{
    env->exception = RISCV_EXCP_ILLEGAL_INST;
    env->pc = insn->pc;
    /* longjmp */
}

void do_ecall(state_t *env, insn_t *insn)
{
    env->exception = RISCV_EXCP_U_ECALL;
    env->pc = insn->pc;
}

/* RV32I */
void do_auipc(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = insn->pc + (target_long) insn->imm;
}

void do_lui(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) insn->imm;
}

void do_jal(state_t *env, insn_t *insn)
{
    if (insn->rd) {
        env->gpr[insn->rd] = insn->pc + 4;
    }
    env->pc = insn->pc + (target_long) insn->imm;
}

void do_jalr(state_t *env, insn_t *insn)
{
    if (insn->rd) {
        env->gpr[insn->rd] = insn->pc + 4;
    }
    env->pc = (env->gpr[insn->rs1] + (target_long) insn->imm) & -2;
}

void do_c_jal(state_t *env, insn_t *insn)
{
    if (insn->rd) {
        env->gpr[insn->rd] = insn->pc + 2;
    }
    env->pc = insn->pc + (target_long) insn->imm;
}

void do_c_jalr(state_t *env, insn_t *insn)
{
    if (insn->rd) {
        env->gpr[insn->rd] = insn->pc + 2;
    }
    env->pc = (env->gpr[insn->rs1] + (target_long) insn->imm) & -2;
}

void do_add(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = env->gpr[insn->rs1] + env->gpr[insn->rs2];
}

void do_sub(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = env->gpr[insn->rs1] - env->gpr[insn->rs2];
}

void do_sll(state_t *env, insn_t *insn)
{
    unsigned int shift = env->gpr[insn->rs2] & (TARGET_LONG_BITS - 1);
    env->gpr[insn->rd] = env->gpr[insn->rs1] << shift;
}

void do_slt(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) env->gpr[insn->rs1] < (target_long) env->gpr[insn->rs2] ? 1 : 0;
}

void do_sltu(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = env->gpr[insn->rs1] < env->gpr[insn->rs2] ? 1 : 0;
}

void do_xor(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = env->gpr[insn->rs1] ^ env->gpr[insn->rs2];
}

void do_srl(state_t *env, insn_t *insn)
{
    unsigned int shift = env->gpr[insn->rs2] & (TARGET_LONG_BITS - 1);
    env->gpr[insn->rd] = env->gpr[insn->rs1] >> shift;
}

void do_sra(state_t *env, insn_t *insn)
{
    unsigned int shift = env->gpr[insn->rs2] & (TARGET_LONG_BITS - 1);
    env->gpr[insn->rd] = (target_long) env->gpr[insn->rs1] >> shift;
}

void do_or(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = env->gpr[insn->rs1] | env->gpr[insn->rs2];
}

void do_and(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = env->gpr[insn->rs1] & env->gpr[insn->rs2];
}

void do_beq(state_t *env, insn_t *insn)
{
    env->pc = insn->pc + (env->gpr[insn->rs1] == env->gpr[insn->rs2] ? (target_long) insn->imm : 4);
}

void do_bne(state_t *env, insn_t *insn)
{
    env->pc = insn->pc + (env->gpr[insn->rs1] != env->gpr[insn->rs2] ? (target_long) insn->imm : 4);
}

void do_blt(state_t *env, insn_t *insn)
{
    env->pc = insn->pc + ((target_long) env->gpr[insn->rs1] < (target_long) env->gpr[insn->rs2] ? (target_long) insn->imm : 4);
}

void do_bge(state_t *env, insn_t *insn)
{
    env->pc = insn->pc + ((target_long) env->gpr[insn->rs1] >= (target_long) env->gpr[insn->rs2] ? (target_long) insn->imm : 4);
}

void do_bltu(state_t *env, insn_t *insn)
{
    env->pc = insn->pc + (env->gpr[insn->rs1] < env->gpr[insn->rs2] ? (target_long) insn->imm : 4);
}

void do_bgeu(state_t *env, insn_t *insn)
{
    env->pc = insn->pc + (env->gpr[insn->rs1] >= env->gpr[insn->rs2] ? (target_long) insn->imm : 4);
}

void do_c_beq(state_t *env, insn_t *insn)
{
    env->pc = insn->pc + (env->gpr[insn->rs1] == env->gpr[insn->rs2] ? (target_long) insn->imm : 2);
}

void do_c_bne(state_t *env, insn_t *insn)
{
    env->pc = insn->pc + (env->gpr[insn->rs1] != env->gpr[insn->rs2] ? (target_long) insn->imm : 2);
}

void do_lb(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) (int8_t) cpu_ldub(env->gpr[insn->rs1], insn->imm);
}

void do_lh(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) (int16_t) cpu_lduh(env->gpr[insn->rs1], insn->imm);
}

void do_lw(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) (int32_t) cpu_lduw(env->gpr[insn->rs1], insn->imm);
}

void do_lbu(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = cpu_ldub(env->gpr[insn->rs1], insn->imm);
}

void do_lhu(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = cpu_lduh(env->gpr[insn->rs1], insn->imm);
}

void do_sb(state_t *env, insn_t *insn)
{
    cpu_stub(env->gpr[insn->rs1], insn->imm, env->gpr[insn->rs2]);
}

void do_sh(state_t *env, insn_t *insn)
{
    cpu_stuh(env->gpr[insn->rs1], insn->imm, env->gpr[insn->rs2]);
}

void do_sw(state_t *env, insn_t *insn)
{
    cpu_stuw(env->gpr[insn->rs1], insn->imm, env->gpr[insn->rs2]);
}

void do_addi(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = env->gpr[insn->rs1] + (target_long) insn->imm;
}

void do_slti(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) env->gpr[insn->rs1] < (target_long) insn->imm ? 1 : 0;
}

void do_sltiu(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_ulong) env->gpr[insn->rs1] < (target_ulong) insn->imm ? 1 : 0;
}

void do_xori(state_t *env, insn_t *insn)
{
     env->gpr[insn->rd] = env->gpr[insn->rs1] ^ (target_long) insn->imm;
}

void do_ori(state_t *env, insn_t *insn)
{
     env->gpr[insn->rd] = env->gpr[insn->rs1] | (target_long) insn->imm;
}

void do_andi(state_t *env, insn_t *insn)
{
     env->gpr[insn->rd] = env->gpr[insn->rs1] & (target_long) insn->imm;
}

void do_slli(state_t *env, insn_t *insn)
{
    unsigned int shift = insn->imm & (TARGET_LONG_BITS - 1);
    env->gpr[insn->rd] = env->gpr[insn->rs1] << shift;
}

void do_srli(state_t *env, insn_t *insn)
{
    unsigned int shift = insn->imm & (TARGET_LONG_BITS - 1);
    env->gpr[insn->rd] = env->gpr[insn->rs1] >> shift;
}

void do_srai(state_t *env, insn_t *insn)
{
    unsigned int shift = insn->imm & (TARGET_LONG_BITS - 1);
    env->gpr[insn->rd] = (target_long) env->gpr[insn->rs1] >> shift;
}

/* RV64I */
void do_lwu(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = cpu_lduw(env->gpr[insn->rs1], insn->imm);
}

void do_ld(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) (int64_t) cpu_ldud(env->gpr[insn->rs1], insn->imm);
}

void do_sd(state_t *env, insn_t *insn)
{
    cpu_stud(env->gpr[insn->rs1], insn->imm, env->gpr[insn->rs2]);
}

void do_addiw(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) ((int32_t) (env->gpr[insn->rs1] + insn->imm));
}

void do_slliw(state_t *env, insn_t *insn)
{
    unsigned int shift = insn->imm & 31;
    env->gpr[insn->rd] = (target_long) ((uint32_t) env->gpr[insn->rs1] << shift);
}

void do_srliw(state_t *env, insn_t *insn)
{
    unsigned int shift = insn->imm & 31;
    env->gpr[insn->rd] = (target_long) ((uint32_t) env->gpr[insn->rs1] >> shift);
}

void do_sraiw(state_t *env, insn_t *insn)
{
    unsigned int shift = insn->imm & 31;
    env->gpr[insn->rd] = (target_long) ((int32_t) env->gpr[insn->rs1] >> shift);
}

void do_addw(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) (int32_t) (env->gpr[insn->rs1] + env->gpr[insn->rs2]);
}

void do_subw(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) (int32_t) (env->gpr[insn->rs1] - env->gpr[insn->rs2]);
}

void do_sllw(state_t *env, insn_t *insn)
{
    unsigned int shift = env->gpr[insn->rs2] & 31;
    env->gpr[insn->rd] = (target_long) ((uint32_t) env->gpr[insn->rs1] << shift);
}

void do_srlw(state_t *env, insn_t *insn)
{
    unsigned int shift = env->gpr[insn->rs2] & 31;
    env->gpr[insn->rd] = (target_long) ((uint32_t) env->gpr[insn->rs1] >> shift);
}

void do_sraw(state_t *env, insn_t *insn)
{
    unsigned int shift = env->gpr[insn->rs2] & 31;
    env->gpr[insn->rd] = (target_long) ((int32_t) env->gpr[insn->rs1] >> shift);
}

/* RV32M */
void wmulu(target_ulong x, target_ulong y, target_ulong *hi, target_ulong *lo)
{
    const uint32_t half_long_bits = TARGET_LONG_BITS >> 1;
    const target_ulong x0 = (target_hulong)x, x1 = x >> half_long_bits;
    const target_ulong y0 = (target_hulong)y, y1 = y >> half_long_bits;
    const target_ulong p11 = x1 * y1, p01 = x0 * y1;
    const target_ulong p10 = x1 * y0, p00 = x0 * y0;
    const target_ulong c = (target_ulong) ((p00 >> half_long_bits) + (target_hulong) p10 + (target_hulong) p01) >> half_long_bits;
    *lo = p00 + (p10 << half_long_bits) + (p01 << half_long_bits);
    *hi = p11 + (p10 >> half_long_bits) + (p01 >> half_long_bits) + c;
}

void wmul(target_long x, target_long y, target_long *hi, target_long *lo)
{
    wmulu((target_ulong)x, (target_ulong)y, (target_ulong *)hi, (target_ulong *)lo);
    if (x < 0LL) {
        *hi -= y;
    }
    if (y < 0LL) {
        *hi -= x;
    }
}

void wmulsu(target_long x, target_ulong y, target_long *hi, target_long *lo)
{
    wmulu((target_ulong)x, (target_ulong)y, (target_ulong *)hi, (target_ulong *)lo);
    if (x < 0LL) {
        *hi -= y;
    }
}

void do_mul(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) env->gpr[insn->rs1] * (target_long) env->gpr[insn->rs2];
}

void do_mulh(state_t *env, insn_t *insn)
{
    target_ulong lo;
    wmul((target_long) env->gpr[insn->rs1], (target_long) env->gpr[insn->rs2],
            (target_long *) &env->gpr[insn->rd], (target_long *) &lo);
}

void do_mulhsu(state_t *env, insn_t *insn)
{
    target_ulong lo;
    wmulsu((target_long) env->gpr[insn->rs1], env->gpr[insn->rs2],
            (target_long *) &env->gpr[insn->rd], (target_long *) &lo);
}

void do_mulhu(state_t *env, insn_t *insn)
{
    target_ulong lo;
    wmulu(env->gpr[insn->rs1], env->gpr[insn->rs2], &env->gpr[insn->rd], &lo);
}

void do_div(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) env->gpr[insn->rs1] / (target_long) env->gpr[insn->rs2];
}

void do_divu(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_ulong) env->gpr[insn->rs1] / (target_ulong) env->gpr[insn->rs2];
}

void do_rem(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) env->gpr[insn->rs1] % (target_long) env->gpr[insn->rs2];
}

void do_remu(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_ulong) env->gpr[insn->rs1] % (target_ulong) env->gpr[insn->rs2];
}

/* RV64M */
void do_mulw(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) (int32_t) (env->gpr[insn->rs1] * env->gpr[insn->rs2]);
}

void do_divw(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) ((int32_t) env->gpr[insn->rs1] / (int32_t) env->gpr[insn->rs2]);
}

void do_divuw(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) ((uint32_t) env->gpr[insn->rs1] / (uint32_t) env->gpr[insn->rs2]);
}

void do_remw(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) ((int32_t) env->gpr[insn->rs1] % (int32_t) env->gpr[insn->rs2]);
}

void do_remuw(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) ((uint32_t) env->gpr[insn->rs1] % (uint32_t) env->gpr[insn->rs2]);
}

static int read_csr(state_t *env, int csr, target_ulong *ret_value)
{
    assert(ret_value);

    switch (csr) {
    case CSR_FFLAGS:
        *ret_value = env->fflags;
        break;
    case CSR_FRM:
        *ret_value = env->frm;
        break;
    case CSR_FCSR:
        *ret_value = env->fcsr;
        break;
    case CSR_VSTART:
        *ret_value = env->vstart;
        break;
    case CSR_VXSAT:
        *ret_value = env->vxsat;
        break;
    case CSR_VXRM:
        *ret_value = env->vxrm;
        break;
    case CSR_VCSR:
        *ret_value = env->vcsr;
        break;
    case CSR_VL:
        *ret_value = env->vl;
        break;
    case CSR_VTYPE:
        *ret_value = env->vtype;
        break;
    case CSR_VLENB:
        *ret_value = env->vlenb;
        break;
    default:
        return 1;
    }
    return 0;
}

static int write_csr(state_t *env, int csr, target_ulong value)
{
    switch (csr) {
    case CSR_FFLAGS:
        env->fflags = value;
        break;
    case CSR_FRM:
        env->frm = value;
        break;
    case CSR_FCSR:
        env->fcsr = value;
        break;
    case CSR_VSTART:
        env->vstart = value;
        break;
    case CSR_VXSAT:
        env->vxsat = value;
        break;
    case CSR_VXRM:
        env->vxrm = value;
        break;
    case CSR_VCSR:
        env->vcsr = value;
        break;
    case CSR_VL:
        env->vl = value;
        break;
    case CSR_VTYPE:
        env->vtype = value;
        break;
    case CSR_VLENB:
        env->vlenb = value;
        break;
    default:
        return 1;
    }
    return 0;
}

static int rmw_csr(state_t *env, int csr, target_ulong *ret_value,
            target_ulong new_value, target_ulong wr_mask)
{
    target_ulong value;

    if (read_csr(env, csr, &value)) {
        return 1;
    }

    new_value = (value & ~wr_mask) | (new_value & wr_mask);
    if (write_csr(env, csr, new_value)) {
        return 1;
    }

    if (ret_value) {
        *ret_value = value;
    }

    return 0;
}

void do_csrrw(state_t *env, insn_t *insn)
{
    if (rmw_csr(env, insn->csr, &env->gpr[insn->rd], env->gpr[insn->rs1], -1)) {
        do_illegal(env, insn);
    }
}

void do_csrrs(state_t *env, insn_t *insn)
{
    if (rmw_csr(env, insn->csr, &env->gpr[insn->rd], -1, env->gpr[insn->rs1])) {
        do_illegal(env, insn);
    }
}

void do_csrrc(state_t *env, insn_t *insn)
{
    if (rmw_csr(env, insn->csr, &env->gpr[insn->rd], 0, env->gpr[insn->rs1])) {
        do_illegal(env, insn);
    }
}

void do_csrrwi(state_t *env, insn_t *insn)
{
    if (rmw_csr(env, insn->csr, &env->gpr[insn->rd], insn->rs1, -1)) {
        do_illegal(env, insn);
    }
}

void do_csrrsi(state_t *env, insn_t *insn)
{
    if (rmw_csr(env, insn->csr, &env->gpr[insn->rd], -1, insn->rs1)) {
        do_illegal(env, insn);
    }
}

void do_csrrci(state_t *env, insn_t *insn)
{
    if (rmw_csr(env, insn->csr, &env->gpr[insn->rd], 0, insn->rs1)) {
        do_illegal(env, insn);
    }
}

/* RV32A */
void do_lr_w(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) (int32_t) cpu_lduw(env->gpr[insn->rs1], 0);
}

void do_sc_w(state_t *env, insn_t *insn)
{
    cpu_stuw(env->gpr[insn->rs1], 0, env->gpr[insn->rs2]);
    env->gpr[insn->rd] = 0;
}

void do_amoadd_w(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) atomic_add((int32_t *) g2h(env->gpr[insn->rs1]), env->gpr[insn->rs2]);
}

void do_amoswap_w(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) atomic_xchg((int32_t *) g2h(env->gpr[insn->rs1]), env->gpr[insn->rs2]);
}

void do_amoxor_w(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) atomic_xor((int32_t *) g2h(env->gpr[insn->rs1]), env->gpr[insn->rs2]);
}

void do_amoor_w(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) atomic_or((int32_t *) g2h(env->gpr[insn->rs1]), env->gpr[insn->rs2]);
}

void do_amoand_w(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) atomic_and((int32_t *) g2h(env->gpr[insn->rs1]), env->gpr[insn->rs2]);
}

void do_amomin_w(state_t *env, insn_t *insn)
{
    int32_t n = (int32_t) cpu_lduw(env->gpr[insn->rs1], 0);
    n = min(n, (int32_t) env->gpr[insn->rs2]);

    env->gpr[insn->rd] = (target_long) atomic_xchg((int32_t *) g2h(env->gpr[insn->rs1]), n);
}

void do_amomax_w(state_t *env, insn_t *insn)
{
    int32_t n = (int32_t) cpu_lduw(env->gpr[insn->rs1], 0);
    n = max(n, (int32_t) env->gpr[insn->rs2]);

    env->gpr[insn->rd] = (target_long) atomic_xchg((int32_t *) g2h(env->gpr[insn->rs1]), n);
}

void do_amominu_w(state_t *env, insn_t *insn)
{
    uint32_t n = (uint32_t) cpu_lduw(env->gpr[insn->rs1], 0);
    n = min(n, (uint32_t) env->gpr[insn->rs2]);

    env->gpr[insn->rd] = (target_long) atomic_xchg((int32_t *) g2h(env->gpr[insn->rs1]), n);
}

void do_amomaxu_w(state_t *env, insn_t *insn)
{
    uint32_t n = (uint32_t) cpu_lduw(env->gpr[insn->rs1], 0);
    n = max(n, (uint32_t) env->gpr[insn->rs2]);

    env->gpr[insn->rd] = (target_long) atomic_xchg((int32_t *) g2h(env->gpr[insn->rs1]), n);
}

/* RV64A */
void do_lr_d(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) (int64_t) cpu_ldud(env->gpr[insn->rs1], 0);
}

void do_sc_d(state_t *env, insn_t *insn)
{
    cpu_stud(env->gpr[insn->rs1], 0, env->gpr[insn->rs2]);
    env->gpr[insn->rd] = 0;
}

void do_amoadd_d(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) atomic_add((int64_t *) g2h(env->gpr[insn->rs1]), env->gpr[insn->rs2]);
}

void do_amoswap_d(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) atomic_xchg((int64_t *) g2h(env->gpr[insn->rs1]), env->gpr[insn->rs2]);
}

void do_amoxor_d(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) atomic_xor((int64_t *) g2h(env->gpr[insn->rs1]), env->gpr[insn->rs2]);
}

void do_amoor_d(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) atomic_or((int64_t *) g2h(env->gpr[insn->rs1]), env->gpr[insn->rs2]);
}

void do_amoand_d(state_t *env, insn_t *insn)
{
    env->gpr[insn->rd] = (target_long) atomic_and((int64_t *) g2h(env->gpr[insn->rs1]), env->gpr[insn->rs2]);
}

void do_amomin_d(state_t *env, insn_t *insn)
{
    int64_t n = (int64_t) cpu_ldud(env->gpr[insn->rs1], 0);
    n = min(n, (int64_t) env->gpr[insn->rs2]);

    env->gpr[insn->rd] = (target_long) atomic_xchg((int64_t *) g2h(env->gpr[insn->rs1]), n);
}

void do_amomax_d(state_t *env, insn_t *insn)
{
    int64_t n = (int64_t) cpu_ldud(env->gpr[insn->rs1], 0);
    n = max(n, (int64_t) env->gpr[insn->rs2]);

    env->gpr[insn->rd] = (target_long) atomic_xchg((int64_t *) g2h(env->gpr[insn->rs1]), n);
}

void do_amominu_d(state_t *env, insn_t *insn)
{
    uint64_t n = (uint64_t) cpu_ldud(env->gpr[insn->rs1], 0);
    n = min(n, (uint64_t) env->gpr[insn->rs2]);

    env->gpr[insn->rd] = (target_long) atomic_xchg((int64_t *) g2h(env->gpr[insn->rs1]), n);
}

void do_amomaxu_d(state_t *env, insn_t *insn)
{
    uint64_t n = (uint64_t) cpu_ldud(env->gpr[insn->rs1], 0);
    n = max(n, (uint64_t) env->gpr[insn->rs2]);

    env->gpr[insn->rd] = (target_long) atomic_xchg((int64_t *) g2h(env->gpr[insn->rs1]), n);
}

#define INSN_BUFFER_MAP_START 0x500000000000
#define INSN_BUFFER_SIZE      1 * GiB
void *insn_buffer_ptr = (void *) INSN_BUFFER_MAP_START;
int alloc_insn_buffer()
{
    int prot = PROT_READ | PROT_WRITE;
    insn_buffer_ptr = host_mmap(insn_buffer_ptr, INSN_BUFFER_SIZE, prot,
#ifndef __STRICT_ANSI__
            MAP_ANONYMOUS |
#endif
            MAP_FIXED | MAP_PRIVATE, -1, 0, NULL);
    if (insn_buffer_ptr == MAP_FAILED) {
        return 0;
    }
    return 1;
}

/* https://en.wikipedia.org/wiki/Hash_function#Fibonacci_hashing */
#define FAST_TABLE_SIZE 4096
#define LOG2_FAST_TABLE_SIZE 12
#define FIBONACCI_HASH_64 11400714819323198485ULL
static trans_block_t *block_hash_table[FAST_TABLE_SIZE] = {0};

static inline unsigned long fibhash(unsigned long k)
{
    k ^= k >> (64 - LOG2_FAST_TABLE_SIZE);
    return (FIBONACCI_HASH_64 * k) >> (64 - LOG2_FAST_TABLE_SIZE);
}

static trans_block_t *find_block(target_ulong pc)
{
    unsigned long hash = fibhash(pc);
    trans_block_t *block = block_hash_table[hash];
    for (; block; block = block->hash_next) {
        if (block->pc == pc) {
            logoutf("[vk1] cached block found pc: 0x%016lx\n", pc);
            return block;
        }
    }
    return NULL;
}

static trans_block_t *new_block(target_ulong pc)
{
    unsigned long hash;
    trans_block_t *block;
    trans_block_t *new_block = malloc(sizeof(*new_block));

    new_block->pc = pc;
    // new_block->insn = malloc(TRANS_MAX * sizeof (insn_t));
    new_block->insn = insn_buffer_ptr;
    // insn_buffer_ptr = insn_buffer_ptr + ROUND_UP(TRANS_MAX * sizeof (insn_t), 64);

    new_block->count = 0;
    new_block->jmp_next[0].pc = 0;
    new_block->jmp_next[0].blk = NULL;
    new_block->jmp_next[1].pc = 0;
    new_block->jmp_next[1].blk = NULL;
    new_block->hash_next = NULL;

    hash = fibhash(pc);
    block = block_hash_table[hash];
    if (block == NULL) {
        block_hash_table[hash] = new_block;
        return new_block;
    }

    for (; block->hash_next; block = block->hash_next);
    block->hash_next = new_block;

    return new_block;
}

static trans_block_t *block_exec(state_t *env, trans_block_t *b)
{
    int i, n;
    trans_block_t *b1 = NULL;
    logoutf("[vk1]   first block exec pc: 0x%016lx\n", b->pc);
    for (; b;) {
        n = b->count;
        for (i = 0; i < n; i++) {
            b->insn[i].handler(env, &b->insn[i]);
            assert(env->gpr[0] == 0);
        }
        b1 = b;
        b = env->pc == b->jmp_next[0].pc ? b->jmp_next[0].blk : b->jmp_next[1].blk;
        if (b) {
            logoutf("[vk1]    next block exec pc: 0x%016lx\n", b->pc);
        }
    }
    return b1;
}

static void patch_block(trans_block_t *prev, trans_block_t *now)
{
    if (prev->jmp_next[0].pc == now->pc) {
        prev->jmp_next[0].blk = now;
    } else if (prev->jmp_next[1].pc == now->pc) {
        prev->jmp_next[1].blk = now;
    }
}

static int cpu_exec(state_t *env)
{
    trans_block_t *block;
    trans_block_t *prev_block = NULL;

    while (env->exception == RISCV_EXCP_NONE) {
        block = find_block(env->pc);
        if (block == NULL) {
            block = new_block(env->pc);
            translate(block);
        }

        if (prev_block) {
            patch_block(prev_block, block);
            prev_block = NULL;
        }

        prev_block = block_exec(env, block);
    }

    return env->exception;
}

void cpu_loop(state_t *env)
{
    int excpnr = 0;
    target_ulong ret;

    if (!alloc_insn_buffer()) {
        fprintf(stderr, "Allocate instructions buffer failed.");
        return;
    }

    env->exception = RISCV_EXCP_NONE;
    while (1) {
        excpnr = cpu_exec(env);
        switch (excpnr) {
        case RISCV_EXCP_U_ECALL:
            logoutf("ecall, pc: 0x%016lx, a7: %lu\n", env->pc, env->gpr[17]);
            ret = do_syscall(env,
                       env->gpr[xA7], env->gpr[xA0], env->gpr[xA1],
                       env->gpr[xA2], env->gpr[xA3], env->gpr[xA4],
                       env->gpr[xA5], 0, 0);
            env->pc += 4;
            env->gpr[xA0] = ret;
            logoutf("ecall ret: 0x%016lx\n", ret);
            break;
        case RISCV_EXCP_ILLEGAL_INST:
            logoutf("Illegal instruction, pc: 0x%016lx\n", env->pc);
            _exit(1);
            goto exit;
            break;
        default:
            assert_not_reached();
            break;
        }
        env->exception = RISCV_EXCP_NONE;
    }
exit:
    return;
}

