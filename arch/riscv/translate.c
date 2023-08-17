#include "../../include/rvvk.h"

typedef struct {
    target_ulong pc;
    uint32_t count;
    int link;
    target_ulong *pc_next[2];
    int exit;
} trans_t;

#define OPCODE_MASK 0x0000007f
#define FUNCT3_MASK 0x00007000
#define FUNCT7_MASK 0xfe000000
#define OP_27_31_MASK 0xf8000000
#define OP_20_24_MASK 0x01f00000

#ifdef CONFIG_TARGET_RISCV64
# define I_11_X_MASK 0xfc000000 /* rv64 supported */
#else
# define I_11_X_MASK 0xfe000000 /* rv32 supported */
#endif

#define EX_SH(amount) \
    static int ex_shift_##amount(trans_t *ctx, int imm) \
    {                                                   \
        return imm << amount;                           \
    }
EX_SH(1)
EX_SH(2)
EX_SH(3)
EX_SH(4)
EX_SH(12)

static int ex_rvc_register(trans_t *ctx, int reg)
{
    return 8 + reg;
}



static void trans_illegal(trans_t *ctx, insn_t *insn, uint32_t op)
{
    insn->handler = do_illegal;
    insn->pc = ctx->pc;
    ctx->exit = 1;
}

static void decode_insn16_extract_c_addi4spn(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_2(ctx, deposit32(deposit32(deposit32(extract32(op, 6, 1), 1, 31, extract32(op, 5, 1)), 2, 30, extract32(op, 11, 2)), 4, 28, extract32(op, 7, 4)));
    insn->rd = ex_rvc_register(ctx, extract32(op, 2, 3));
    insn->rs1 = 2;
}

static void decode_insn16_extract_c_lw(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_2(ctx, deposit32(deposit32(extract32(op, 6, 1), 1, 31, extract32(op, 10, 3)), 4, 28, extract32(op, 5, 1)));
    insn->rd = ex_rvc_register(ctx, extract32(op, 2, 3));
    insn->rs1 = ex_rvc_register(ctx, extract32(op, 7, 3));
}

static void decode_insn16_extract_c_sw(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_2(ctx, deposit32(deposit32(extract32(op, 6, 1), 1, 31, extract32(op, 10, 3)), 4, 28, extract32(op, 5, 1)));
    insn->rs2 = ex_rvc_register(ctx, extract32(op, 2, 3));
    insn->rs1 = ex_rvc_register(ctx, extract32(op, 7, 3));
}

static void decode_insn16_extract_c_ld(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_3(ctx, deposit32(extract32(op, 10, 3), 3, 29, extract32(op, 5, 2)));
    insn->rd = ex_rvc_register(ctx, extract32(op, 2, 3));
    insn->rs1 = ex_rvc_register(ctx, extract32(op, 7, 3));
}

static void decode_insn16_extract_c_sd(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_3(ctx, deposit32(extract32(op, 10, 3), 3, 29, extract32(op, 5, 2)));
    insn->rs2 = ex_rvc_register(ctx, extract32(op, 2, 3));
    insn->rs1 = ex_rvc_register(ctx, extract32(op, 7, 3));
}

static void decode_insn16_extract_c_addi(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = deposit32(extract32(op, 2, 5), 5, 27, sextract32(op, 12, 1));
    insn->rd = extract32(op, 7, 5);
    insn->rs1 = extract32(op, 7, 5);
}
#define decode_insn16_extract_c_addiw decode_insn16_extract_c_addi

#ifdef CONFIG_TARGET_RISCV32
static void decode_insn16_extract_c_jal(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_1(ctx, deposit32(deposit32(deposit32(deposit32(deposit32(deposit32(deposit32(extract32(op, 3, 3), 3, 29, extract32(op, 11, 1)), 4, 28, extract32(op, 2, 1)), 5, 27, extract32(op, 7, 1)), 6, 26, extract32(op, 6, 1)), 7, 25, extract32(op, 9, 2)), 9, 23, extract32(op, 8, 1)), 10, 22, sextract32(op, 12, 1)));
    insn->rd = 1;
}
#endif

static void decode_insn16_extract_c_li(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = deposit32(extract32(op, 2, 5), 5, 27, sextract32(op, 12, 1));
    insn->rd = extract32(op, 7, 5);
    insn->rs1 = 0;
}

static void decode_insn16_extract_c_addi16sp(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_4(ctx, deposit32(deposit32(deposit32(deposit32(extract32(op, 6, 1), 1, 31, extract32(op, 2, 1)), 2, 30, extract32(op, 5, 1)), 3, 29, extract32(op, 3, 2)), 5, 27, sextract32(op, 12, 1)));
    insn->rd = 2;
    insn->rs1 = 2;
}

static void decode_insn16_extract_c_lui(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_12(ctx, deposit32(extract32(op, 2, 5), 5, 27, sextract32(op, 12, 1)));
    insn->rd = extract32(op, 7, 5);
}

static void decode_insn16_extract_c_srli(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->rs1 = ex_rvc_register(ctx, extract32(op, 7, 3));
    insn->imm = deposit32(extract32(op, 2, 5), 5, 27, extract32(op, 12, 1));
    insn->rd = ex_rvc_register(ctx, extract32(op, 7, 3));
}
#define decode_insn16_extract_c_srai decode_insn16_extract_c_srli

static void decode_insn16_extract_c_andi(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->rs1 = ex_rvc_register(ctx, extract32(op, 7, 3));
    insn->imm = deposit32(extract32(op, 2, 5), 5, 27, sextract32(op, 12, 1));
    insn->rd = ex_rvc_register(ctx, extract32(op, 7, 3));
}

static void decode_insn16_extract_c_sub(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->rs2 = ex_rvc_register(ctx, extract32(op, 2, 3));
    insn->rd = ex_rvc_register(ctx, extract32(op, 7, 3));
    insn->rs1 = ex_rvc_register(ctx, extract32(op, 7, 3));
}
#define decode_insn16_extract_c_xor decode_insn16_extract_c_sub
#define decode_insn16_extract_c_or decode_insn16_extract_c_sub
#define decode_insn16_extract_c_and decode_insn16_extract_c_sub
#define decode_insn16_extract_c_subw decode_insn16_extract_c_sub
#define decode_insn16_extract_c_addw decode_insn16_extract_c_sub

static void decode_insn16_extract_c_j(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_1(ctx, deposit32(deposit32(deposit32(deposit32(deposit32(deposit32(deposit32(extract32(op, 3, 3), 3, 29, extract32(op, 11, 1)), 4, 28, extract32(op, 2, 1)), 5, 27, extract32(op, 7, 1)), 6, 26, extract32(op, 6, 1)), 7, 25, extract32(op, 9, 2)), 9, 23, extract32(op, 8, 1)), 10, 22, sextract32(op, 12, 1)));
    insn->rd = 0;
}

static void decode_insn16_extract_c_beqz(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_1(ctx, deposit32(deposit32(deposit32(deposit32(extract32(op, 3, 2), 2, 30, extract32(op, 10, 2)), 4, 28, extract32(op, 2, 1)), 5, 27, extract32(op, 5, 2)), 7, 25, sextract32(op, 12, 1)));
    insn->rs2 = 0;
    insn->rs1 = ex_rvc_register(ctx, extract32(op, 7, 3));
}
#define decode_insn16_extract_c_bnez decode_insn16_extract_c_beqz

static void decode_insn16_extract_c_mv(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = 0;
    insn->rd = extract32(op, 7, 5);
    insn->rs1 = extract32(op, 2, 5);
}

static void decode_insn16_extract_c_ldsp(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_3(ctx, deposit32(deposit32(extract32(op, 5, 2), 2, 30, extract32(op, 12, 1)), 3, 29, extract32(op, 2, 3)));
    insn->rd = extract32(op, 7, 5);
    insn->rs1 = 2;
}

static void decode_insn16_extract_c_slli(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->rs1 = extract32(op, 7, 5);
    insn->imm = deposit32(extract32(op, 2, 5), 5, 27, extract32(op, 12, 1));
    insn->rd = extract32(op, 7, 5);
}

static void decode_insn16_extract_c_lwsp(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_2(ctx, deposit32(deposit32(extract32(op, 4, 3), 3, 29, extract32(op, 12, 1)), 4, 28, extract32(op, 2, 2)));
    insn->rd = extract32(op, 7, 5);
    insn->rs1 = 2;
}

static void decode_insn16_extract_c_jalr(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->rd = 1;
    insn->imm = 0;
    insn->rs1 = extract32(op, 7, 5);
}

static void decode_insn16_extract_c_jr(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->rd = 0;
    insn->imm = 0;
    insn->rs1 = extract32(op, 7, 5);
}

static void decode_insn16_extract_c_add(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->rs2 = extract32(op, 2, 5);
    insn->rd = extract32(op, 7, 5);
    insn->rs1 = extract32(op, 7, 5);
}

static void decode_insn16_extract_c_swsp(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_2(ctx, deposit32(extract32(op, 9, 4), 4, 28, extract32(op, 7, 2)));
    insn->rs2 = extract32(op, 2, 5);
    insn->rs1 = 2;
}

static void decode_insn16_extract_c_sdsp(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->imm = ex_shift_3(ctx, deposit32(extract32(op, 10, 3), 3, 29, extract32(op, 7, 3)));
    insn->rs2 = extract32(op, 2, 5);
    insn->rs1 = 2;
}



static void trans_c_addi4spn(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_addi4spn(ctx, insn, op);
    insn->handler = do_addi;
}

static void trans_c_lw(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_lw(ctx, insn, op);
    insn->handler = do_lw;
}

static void trans_c_ld(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_ld(ctx, insn, op);
    insn->handler = do_ld;
}

static void trans_c_sw(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_sw(ctx, insn, op);
    insn->handler = do_sw;
}

static void trans_c_sd(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_sd(ctx, insn, op);
    insn->handler = do_sd;
}

static void trans_c_addi(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_addi(ctx, insn, op);
    /* c.nop */
    if (unlikely(insn->rd == 0)) {
        insn->handler = do_empty;
        return;
    }
    insn->handler = do_addi;
}

#ifdef CONFIG_TARGET_RISCV32
static void trans_c_jal(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_jal(ctx, insn, op);
    insn->handler = do_c_jal;
    insn->pc = ctx->pc;
    ctx->link = 1;
    *ctx->pc_next[0] = ctx->pc + (target_long) insn->imm;
    ctx->exit = 1;
}
#else
static void trans_c_addiw(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_addiw(ctx, insn, op);
    if (unlikely(insn->rd == 0)) {
        trans_illegal(ctx, insn, op);
        return;
    }
    insn->handler = do_addiw;
}
#endif

static void trans_c_li(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_li(ctx, insn, op);
    if (unlikely(insn->rd == 0)) {
        trans_illegal(ctx, insn, op);
        return;
    }
    insn->handler = do_addi;
}

static void trans_c_addi16sp(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_addi16sp(ctx, insn, op);
    insn->handler = do_addi;
}

static void trans_c_lui(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_lui(ctx, insn, op);
    if (unlikely(insn->rd == 0 || insn->rd == 2 || insn->imm == 0)) {
        trans_illegal(ctx, insn, op);
        return;
    }
    insn->handler = do_lui;
}

static void trans_c_srli(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_srli(ctx, insn, op);
    insn->handler = do_srli;
}

static void trans_c_srai(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_srai(ctx, insn, op);
    insn->handler = do_srai;
}

static void trans_c_andi(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_andi(ctx, insn, op);
    insn->handler = do_andi;
}

static void trans_c_sub(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_sub(ctx, insn, op);
    insn->handler = do_sub;
}

static void trans_c_xor(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_xor(ctx, insn, op);
    insn->handler = do_xor;
}

static void trans_c_or(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_or(ctx, insn, op);
    insn->handler = do_or;
}

static void trans_c_and(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_and(ctx, insn, op);
    insn->handler = do_and;
}

static void trans_c_subw(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_subw(ctx, insn, op);
    insn->handler = do_subw;
}

static void trans_c_addw(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_addw(ctx, insn, op);
    insn->handler = do_addw;
}

static void trans_c_j(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_j(ctx, insn, op);
    insn->handler = do_c_jal;
    insn->pc = ctx->pc;
    ctx->link = 1;
    *ctx->pc_next[0] = ctx->pc + (target_long) insn->imm;
    ctx->exit = 1;
}

static void trans_c_beqz(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_beqz(ctx, insn, op);
    insn->handler = do_c_beq;
    insn->pc = ctx->pc;
    ctx->link = 1;
    *ctx->pc_next[0] = ctx->pc + (target_long) insn->imm;
    *ctx->pc_next[1] = ctx->pc + 2;
    ctx->exit = 1;
}

static void trans_c_bnez(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_bnez(ctx, insn, op);
    insn->handler = do_c_bne;
    insn->pc = ctx->pc;
    ctx->link = 1;
    *ctx->pc_next[0] = ctx->pc + (target_long) insn->imm;
    *ctx->pc_next[1] = ctx->pc + 2;
    ctx->exit = 1;
}

static void trans_c_jr(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_jr(ctx, insn, op);
    if (unlikely(insn->rs1 == 0)) {
        trans_illegal(ctx, insn, op);
        return;
    }
    insn->handler = do_c_jalr;
    insn->pc = ctx->pc;
    ctx->exit = 1;
}

static void trans_c_mv(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_mv(ctx, insn, op);
    if (unlikely(insn->rd == 0 || insn->rs1 == 0)) {
        trans_illegal(ctx, insn, op);
        return;
    }
    insn->handler = do_addi;
}

static void trans_c_ldsp(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_ldsp(ctx, insn, op);
    if (unlikely(insn->rd == 0)) {
        trans_illegal(ctx, insn, op);
        return;
    }
    insn->handler = do_ld;
}

static void trans_c_slli(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_slli(ctx, insn, op);
    if (unlikely(insn->rd == 0)) {
        trans_illegal(ctx, insn, op);
        return;
    }
    insn->handler = do_slli;
}

static void trans_c_lwsp(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_lwsp(ctx, insn, op);
    if (unlikely(insn->rd == 0)) {
        trans_illegal(ctx, insn, op);
        return;
    }
    insn->handler = do_lw;
}

static void trans_c_ebreak(trans_t *ctx, insn_t *insn, uint16_t op)
{
    insn->handler = do_empty;
}

static void trans_c_jalr(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_jalr(ctx, insn, op);
    if (unlikely(insn->rs1 == 0)) {
        trans_illegal(ctx, insn, op);
        return;
    }
    insn->handler = do_c_jalr;
    insn->pc = ctx->pc;
    ctx->exit = 1;
}

static void trans_c_add(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_add(ctx, insn, op);
    if (unlikely(insn->rd == 0 || insn->rs2 == 0)) {
        trans_illegal(ctx, insn, op);
        return;
    }
    insn->handler = do_add;
}

static void trans_c_swsp(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_swsp(ctx, insn, op);
    insn->handler = do_sw;
}

static void trans_c_sdsp(trans_t *ctx, insn_t *insn, uint16_t op)
{
    decode_insn16_extract_c_sdsp(ctx, insn, op);
    insn->handler = do_sd;
}



static void decode_insn16(trans_t *ctx, insn_t *insn, uint16_t op)
{
    switch (op & 0xe003) {
    case 0x0000:
        if ((op & 0x1fe0) == 0) {
            trans_illegal(ctx, insn, op);
            return;
        }
        trans_c_addi4spn(ctx, insn, op);
        return;
    case 0x0001:
        trans_c_addi(ctx, insn, op);
        return;
    case 0x0002:
        trans_c_slli(ctx, insn, op);
        return;
    case 0x2001:
#ifdef CONFIG_TARGET_RISCV32
        trans_c_jal(ctx, insn, op);
#else
        trans_c_addiw(ctx, insn, op);
#endif
        return;
    case 0x4000:
        trans_c_lw(ctx, insn, op);
        return;
    case 0x4001:
        trans_c_li(ctx, insn, op);
        return;
    case 0x4002:
        trans_c_lwsp(ctx, insn, op);
        return;
    case 0x6000:
        trans_c_ld(ctx, insn, op);
        return;
    case 0x6001:
        if ((op & 0x0f80) == 0x0100) {
            trans_c_addi16sp(ctx, insn, op);
            return;
        }
        trans_c_lui(ctx, insn, op);
        return;
    case 0x6002:
        trans_c_ldsp(ctx, insn, op);
        return;
    case 0x8001:
        switch ((op >> 10) & 0x3) {
        case 0x0:
            trans_c_srli(ctx, insn, op);
            return;
        case 0x1:
            trans_c_srai(ctx, insn, op);
            return;
        case 0x2:
            trans_c_andi(ctx, insn, op);
            return;
        case 0x3:
            switch (op & 0x1060) {
            case 0x0000:
                trans_c_sub(ctx, insn, op);
                return;
            case 0x0020:
                trans_c_xor(ctx, insn, op);
                return;
            case 0x0040:
                trans_c_or(ctx, insn, op);
                return;
            case 0x0060:
                trans_c_and(ctx, insn, op);
                return;
            case 0x1000:
                trans_c_subw(ctx, insn, op);
                return;
            case 0x1020:
                trans_c_addw(ctx, insn, op);
                return;
            }
            break;
        }
        break;
    case 0x8002:
        switch ((op >> 12) & 0x1) {
        case 0x0:
            if ((op & 0x007c) == 0x0000) {
                trans_c_jr(ctx, insn, op);
                return;
            }
            trans_c_mv(ctx, insn, op);
            return;
        case 0x1:
            if ((op & 0x0ffc) == 0x0000) {
                trans_c_ebreak(ctx, insn, op);
                return;
            }
            if ((op & 0x007c) == 0x0000) {
                trans_c_jalr(ctx, insn, op);
                return;
            }
            trans_c_add(ctx, insn, op);
            return;
        }
        break;
    case 0xa001:
        trans_c_j(ctx, insn, op);
        return;
    case 0xc000:
        trans_c_sw(ctx, insn, op);
        return;
    case 0xc002:
        trans_c_swsp(ctx, insn, op);
        return;
    case 0xc001:
        trans_c_beqz(ctx, insn, op);
        return;
    case 0xe000:
        trans_c_sd(ctx, insn, op);
        return;
    case 0xe001:
        trans_c_bnez(ctx, insn, op);
        return;
    case 0xe002:
        trans_c_sdsp(ctx, insn, op);
        return;
    }
    trans_illegal(ctx, insn, op);
    return;
}

static void decode_insn32_extract_r(trans_t *ctx, insn_t *insn, uint32_t op)
{
    insn->rs1 = extract32(op, 15, 5);
    insn->rd = extract32(op, 7, 5);
    insn->rs2 = extract32(op, 20, 5);
}

static void decode_insn32_extract_i(trans_t *ctx, insn_t *insn, uint32_t op)
{
    insn->rs1 = extract32(op, 15, 5);
    insn->rd = extract32(op, 7, 5);
    insn->imm = sextract32(op, 20, 12);
}

static void decode_insn32_extract_s(trans_t *ctx, insn_t *insn, uint32_t op)
{
    insn->rs1 = extract32(op, 15, 5);
    insn->imm = deposit32(extract32(op, 7, 5), 5, 27, sextract32(op, 25, 7));
    insn->rs2 = extract32(op, 20, 5);
}

static void decode_insn32_extract_b(trans_t *ctx, insn_t *insn, uint32_t op)
{
    insn->rs1 = extract32(op, 15, 5);
    insn->imm = ex_shift_1(ctx, deposit32(deposit32(deposit32(extract32(op, 8, 4), 4, 28, extract32(op, 25, 6)), 10, 22, extract32(op, 7, 1)), 11, 21, sextract32(op, 31, 1)));
    insn->rs2 = extract32(op, 20, 5);
}

static void decode_insn32_extract_u(trans_t *ctx, insn_t *insn, uint32_t op)
{
    insn->rd = extract32(op, 7, 5);
    insn->imm = ex_shift_12(ctx, sextract32(op, 12, 20));
}

static void decode_insn32_extract_j(trans_t *ctx, insn_t *insn, uint32_t op)
{
    insn->rd = extract32(op, 7, 5);
    insn->imm = ex_shift_1(ctx, deposit32(deposit32(deposit32(extract32(op, 21, 10), 10, 22, extract32(op, 20, 1)), 11, 21, extract32(op, 12, 8)), 19, 13, sextract32(op, 31, 1)));
}

static void decode_insn32_extract_csr(trans_t *ctx, insn_t *insn, uint32_t op)
{
    insn->rs1 = extract32(op, 15, 5);
    insn->rd = extract32(op, 7, 5);
    insn->csr = extract32(op, 20, 12);
}

static void decode_insn32_extract_atom_ld(trans_t *ctx, insn_t *insn, uint32_t op)
{
    // insn->rs2 = 0;
    // insn->rl = extract32(op, 25, 1);
    // insn->aq = extract32(op, 26, 1);
    insn->rd = extract32(op, 7, 5);
    insn->rs1 = extract32(op, 15, 5);
}

static void decode_insn32_extract_atom_st(trans_t *ctx, insn_t *insn, uint32_t op)
{
    insn->rs2 = extract32(op, 20, 5);
    // insn->rl = extract32(op, 25, 1);
    // insn->aq = extract32(op, 26, 1);
    insn->rd = extract32(op, 7, 5);
    insn->rs1 = extract32(op, 15, 5);
}
#define decode_insn32_extract_atom_add decode_insn32_extract_atom_st
#define decode_insn32_extract_atom_swap decode_insn32_extract_atom_st
#define decode_insn32_extract_atom_xor decode_insn32_extract_atom_st
#define decode_insn32_extract_atom_or decode_insn32_extract_atom_st
#define decode_insn32_extract_atom_and decode_insn32_extract_atom_st
#define decode_insn32_extract_atom_min decode_insn32_extract_atom_st
#define decode_insn32_extract_atom_max decode_insn32_extract_atom_st
#define decode_insn32_extract_atom_minu decode_insn32_extract_atom_st
#define decode_insn32_extract_atom_maxu decode_insn32_extract_atom_st

#define gen_trans_r(NAME)                                                \
    static void trans_ ## NAME(trans_t *ctx, insn_t *insn, uint32_t op)  \
    {                                                                    \
        decode_insn32_extract_r(ctx, insn, op);                          \
        if (unlikely(insn->rd == 0)) {                                   \
            insn->handler = do_empty;                                    \
            return;                                                      \
        }                                                                \
        insn->handler = do_ ## NAME;                                     \
    }
gen_trans_r(add)
gen_trans_r(sub)
gen_trans_r(sll)
gen_trans_r(slt)
gen_trans_r(sltu)
gen_trans_r(xor)
gen_trans_r(srl)
gen_trans_r(sra)
gen_trans_r(or)
gen_trans_r(and)

gen_trans_r(addw)
gen_trans_r(subw)
gen_trans_r(sllw)
gen_trans_r(srlw)
gen_trans_r(sraw)

gen_trans_r(mul)
gen_trans_r(mulh)
gen_trans_r(mulhsu)
gen_trans_r(mulhu)
gen_trans_r(div)
gen_trans_r(divu)
gen_trans_r(rem)
gen_trans_r(remu)

gen_trans_r(mulw)
gen_trans_r(divw)
gen_trans_r(divuw)
gen_trans_r(remw)
gen_trans_r(remuw)


#define gen_trans_i(NAME)                                                \
    static void trans_ ## NAME(trans_t *ctx, insn_t *insn, uint32_t op)  \
    {                                                                    \
        decode_insn32_extract_i(ctx, insn, op);                          \
        if (unlikely(insn->rd == 0)) {                                   \
            insn->handler = do_empty;                                    \
            return;                                                      \
        }                                                                \
        insn->handler = do_ ## NAME;                                     \
    }
gen_trans_i(lb)
gen_trans_i(lh)
gen_trans_i(lw)
gen_trans_i(ld)
gen_trans_i(lbu)
gen_trans_i(lhu)
gen_trans_i(lwu)

gen_trans_i(addi)
gen_trans_i(slti)
gen_trans_i(sltiu)
gen_trans_i(xori)
gen_trans_i(ori)
gen_trans_i(andi)

gen_trans_i(slli)
gen_trans_i(srli)
gen_trans_i(srai)

gen_trans_i(addiw)
gen_trans_i(slliw)
gen_trans_i(srliw)
gen_trans_i(sraiw)


#define gen_trans_s(NAME)                                                \
    static void trans_ ## NAME(trans_t *ctx, insn_t *insn, uint32_t op)  \
    {                                                                    \
        decode_insn32_extract_s(ctx, insn, op);                          \
        insn->handler = do_ ## NAME;                                     \
    }
gen_trans_s(sb)
gen_trans_s(sh)
gen_trans_s(sw)

gen_trans_s(sd)


#define gen_trans_b(NAME)                                                \
    static void trans_ ## NAME(trans_t *ctx, insn_t *insn, uint32_t op)  \
    {                                                                    \
        decode_insn32_extract_b(ctx, insn, op);                          \
        insn->handler = do_ ## NAME;                                     \
        insn->pc = ctx->pc;                                              \
        ctx->link = 1;                                                   \
        *ctx->pc_next[0] = ctx->pc + (target_long) insn->imm;            \
        *ctx->pc_next[1] = ctx->pc + 4;                                  \
        ctx->exit = 1;                                                   \
    }
gen_trans_b(beq)
gen_trans_b(bne)
gen_trans_b(blt)
gen_trans_b(bge)
gen_trans_b(bltu)
gen_trans_b(bgeu)



static void trans_ecall(trans_t *ctx, insn_t *insn, uint32_t op)
{
    insn->handler = do_ecall;
    insn->pc = ctx->pc;
    ctx->exit = 1;
}

static void trans_auipc(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_u(ctx, insn, op);
    insn->handler = do_auipc;
    insn->pc = ctx->pc;
}

static void trans_lui(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_u(ctx, insn, op);
    insn->handler = do_lui;
}

static void trans_jal(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_j(ctx, insn, op);
    insn->handler = do_jal;
    insn->pc = ctx->pc;
    ctx->link = 1;
    *ctx->pc_next[0] = ctx->pc + (target_long) insn->imm;
    ctx->exit = 1;
}

static void trans_jalr(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_i(ctx, insn, op);
    insn->handler = do_jalr;
    insn->pc = ctx->pc;
    ctx->exit = 1;
}

static void trans_pause(trans_t *ctx, insn_t *insn, uint32_t op)
{
    insn->handler = do_empty;
}

static void trans_fence(trans_t *ctx, insn_t *insn, uint32_t op)
{
    insn->handler = do_empty;
}

static void trans_fence_i(trans_t *ctx, insn_t *insn, uint32_t op)
{
    insn->handler = do_empty;
}

#define gen_trans_csr(NAME)                                              \
    static void trans_ ## NAME(trans_t *ctx, insn_t *insn, uint32_t op)  \
    {                                                                    \
        decode_insn32_extract_csr(ctx, insn, op);                        \
        insn->handler = do_ ## NAME;                                     \
    }
gen_trans_csr(csrrw)
gen_trans_csr(csrrs)
gen_trans_csr(csrrc)
gen_trans_csr(csrrwi)
gen_trans_csr(csrrsi)
gen_trans_csr(csrrci)

static void trans_lr_w(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_ld(ctx, insn, op);
    insn->handler = do_lr_w;
}

static void trans_sc_w(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_st(ctx, insn, op);
    insn->handler = do_sc_w;
}

static void trans_amoadd_w(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_add(ctx, insn, op);
    insn->handler = do_amoadd_w;
}

static void trans_amoswap_w(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_swap(ctx, insn, op);
    insn->handler = do_amoswap_w;
}

static void trans_amoxor_w(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_xor(ctx, insn, op);
    insn->handler = do_amoxor_w;
}

static void trans_amoor_w(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_or(ctx, insn, op);
    insn->handler = do_amoor_w;
}

static void trans_amoand_w(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_and(ctx, insn, op);
    insn->handler = do_amoand_w;
}

static void trans_amomin_w(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_min(ctx, insn, op);
    insn->handler = do_amomin_w;
}

static void trans_amomax_w(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_max(ctx, insn, op);
    insn->handler = do_amomax_w;
}

static void trans_amominu_w(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_minu(ctx, insn, op);
    insn->handler = do_amominu_w;
}

static void trans_amomaxu_w(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_maxu(ctx, insn, op);
    insn->handler = do_amomaxu_w;
}

static void trans_lr_d(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_ld(ctx, insn, op);
    insn->handler = do_lr_d;
}

static void trans_sc_d(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_st(ctx, insn, op);
    insn->handler = do_sc_d;
}

static void trans_amoadd_d(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_add(ctx, insn, op);
    insn->handler = do_amoadd_d;
}

static void trans_amoswap_d(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_swap(ctx, insn, op);
    insn->handler = do_amoswap_d;
}

static void trans_amoxor_d(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_xor(ctx, insn, op);
    insn->handler = do_amoxor_d;
}

static void trans_amoor_d(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_or(ctx, insn, op);
    insn->handler = do_amoor_d;
}

static void trans_amoand_d(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_and(ctx, insn, op);
    insn->handler = do_amoand_d;
}

static void trans_amomin_d(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_min(ctx, insn, op);
    insn->handler = do_amomin_d;
}

static void trans_amomax_d(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_max(ctx, insn, op);
    insn->handler = do_amomax_d;
}

static void trans_amominu_d(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_minu(ctx, insn, op);
    insn->handler = do_amominu_d;
}

static void trans_amomaxu_d(trans_t *ctx, insn_t *insn, uint32_t op)
{
    decode_insn32_extract_atom_maxu(ctx, insn, op);
    insn->handler = do_amomaxu_d;
}


static void decode_insn32(trans_t *ctx, insn_t *insn, uint32_t op)
{
    switch (op & OPCODE_MASK) {
    case 0x00000003:
        switch (op & FUNCT3_MASK) {
        case 0x00000000:
            trans_lb(ctx, insn, op);
            return;
        case 0x00001000:
            trans_lh(ctx, insn, op);
            return;
        case 0x00002000:
            trans_lw(ctx, insn, op);
            return;
        case 0x00003000:
            trans_ld(ctx, insn, op);
            return;
        case 0x00004000:
            trans_lbu(ctx, insn, op);
            return;
        case 0x00005000:
            trans_lhu(ctx, insn, op);
            return;
        case 0x00006000:
            trans_lwu(ctx, insn, op);
            return;
        }
        break;
    case 0x0000000f:
        switch (op & FUNCT3_MASK) {
        case 0x00000000:
            if ((op & 0xffff8f80u) == 0x01000000) {
                trans_pause(ctx, insn, op);
            }
            trans_fence(ctx, insn, op);
            return;
        case 0x00001000:
            trans_fence_i(ctx, insn, op);
            return;
        }
        break;
    case 0x00000013:
        switch (op & FUNCT3_MASK) {
        case 0x00000000:
            trans_addi(ctx, insn, op);
            return;
        case 0x00001000:
            switch (op & I_11_X_MASK) {
            case 0x00000000:
                trans_slli(ctx, insn, op);
                return;
            }
            break;
        case 0x00002000:
            trans_slti(ctx, insn, op);
            return;
        case 0x00003000:
            trans_sltiu(ctx, insn, op);
            return;
        case 0x00004000:
            trans_xori(ctx, insn, op);
            return;
        case 0x00005000:
            switch (op & I_11_X_MASK) {
            case 0x00000000:
                trans_srli(ctx, insn, op);
                return;
            case 0x40000000:
                trans_srai(ctx, insn, op);
                return;
            }
            break;
        case 0x00006000:
            trans_ori(ctx, insn, op);
            return;
        case 0x00007000:
            trans_andi(ctx, insn, op);
            return;
        }
        break;
    case 0x00000017:
        trans_auipc(ctx, insn, op);
        return;
    case 0x0000001b:
        switch (op & FUNCT3_MASK) {
        case 0x00000000:
            trans_addiw(ctx, insn, op);
            return;
        case 0x00001000:
            switch (op & FUNCT7_MASK) {
            case 0x00000000:
                trans_slliw(ctx, insn, op);
                return;
            }
            break;
        case 0x00005000:
            switch (op & FUNCT7_MASK) {
            case 0x00000000:
                trans_srliw(ctx, insn, op);
                return;
            case 0x40000000:
                trans_sraiw(ctx, insn, op);
                return;
            }
            break;
        }
        break;
    case 0x00000023:
        switch (op & FUNCT3_MASK) {
        case 0x00000000:
            trans_sb(ctx, insn, op);
            return;
        case 0x00001000:
            trans_sh(ctx, insn, op);
            return;
        case 0x00002000:
            trans_sw(ctx, insn, op);
            return;
        case 0x00003000:
            trans_sd(ctx, insn, op);
            return;
        }
        break;
    case 0x0000002f:
        switch (op & FUNCT3_MASK) {
        case 0x00002000:
            switch (op & OP_27_31_MASK) {
            case 0x00000000:
                trans_amoadd_w(ctx, insn, op);
                return;
            case 0x08000000:
                trans_amoswap_w(ctx, insn, op);
                return;
            case 0x10000000:
                switch (op & OP_20_24_MASK) {
                case 0x00000000:
                    trans_lr_w(ctx, insn, op);
                    return;
                }
                break;
            case 0x18000000:
                trans_sc_w(ctx, insn, op);
                return;
            case 0x20000000:
                trans_amoxor_w(ctx, insn, op);
                return;
            case 0x40000000:
                trans_amoor_w(ctx, insn, op);
                return;
            case 0x60000000:
                trans_amoand_w(ctx, insn, op);
                return;
            case 0x80000000:
                trans_amomin_w(ctx, insn, op);
                return;
            case 0xa0000000:
                trans_amomax_w(ctx, insn, op);
                return;
            case 0xc0000000:
                trans_amominu_w(ctx, insn, op);
                return;
            case 0xe0000000:
                trans_amomaxu_w(ctx, insn, op);
                return;
            }
            break;
        case 0x00003000:
            switch (op & OP_27_31_MASK) {
            case 0x00000000:
                trans_amoadd_d(ctx, insn, op);
                return;
            case 0x08000000:
                trans_amoswap_d(ctx, insn, op);
                return;
            case 0x10000000:
                switch (op & OP_20_24_MASK) {
                case 0x00000000:
                    trans_lr_d(ctx, insn, op);
                    return;
                }
                break;
            case 0x18000000:
                trans_sc_d(ctx, insn, op);
                return;
            case 0x20000000:
                trans_amoxor_d(ctx, insn, op);
                return;
            case 0x40000000:
                trans_amoor_d(ctx, insn, op);
                return;
            case 0x60000000:
                trans_amoand_d(ctx, insn, op);
                return;
            case 0x80000000:
                trans_amomin_d(ctx, insn, op);
                return;
            case 0xa0000000:
                trans_amomax_d(ctx, insn, op);
                return;
            case 0xc0000000:
                trans_amominu_d(ctx, insn, op);
                return;
            case 0xe0000000:
                trans_amomaxu_d(ctx, insn, op);
                return;
            }
            break;
        }
        break;
    case 0x00000033:
        switch (op & FUNCT3_MASK) {
        case 0x00000000:
            switch (op & FUNCT7_MASK) {
            case 0x00000000:
                trans_add(ctx, insn, op);
                return;
            case 0x40000000:
                trans_sub(ctx, insn, op);
                return;
            case 0x02000000:
                trans_mul(ctx, insn, op);
                return;
            }
            break;
        case 0x00001000:
            switch (op & FUNCT7_MASK) {
            case 0x00000000:
                trans_sll(ctx, insn, op);
                return;
            case 0x02000000:
                trans_mulh(ctx, insn, op);
                return;
            }
            break;
        case 0x00002000:
            switch (op & FUNCT7_MASK) {
            case 0x00000000:
                trans_slt(ctx, insn, op);
                return;
            case 0x02000000:
                trans_mulhsu(ctx, insn, op);
                return;
            }
            break;
        case 0x00003000:
            switch (op & FUNCT7_MASK) {
            case 0x00000000:
                trans_sltu(ctx, insn, op);
                return;
            case 0x02000000:
                trans_mulhu(ctx, insn, op);
                return;
            }
            break;
        case 0x00004000:
            switch (op & FUNCT7_MASK) {
            case 0x00000000:
                trans_xor(ctx, insn, op);
                return;
            case 0x02000000:
                trans_div(ctx, insn, op);
                return;
            }
            break;
        case 0x00005000:
            switch (op & FUNCT7_MASK) {
            case 0x00000000:
                trans_srl(ctx, insn, op);
                return;
            case 0x40000000:
                trans_sra(ctx, insn, op);
                return;
            case 0x02000000:
                trans_divu(ctx, insn, op);
                return;
            }
            break;
        case 0x00006000:
            switch (op & FUNCT7_MASK) {
            case 0x00000000:
                trans_or(ctx, insn, op);
                return;
            case 0x02000000:
                trans_rem(ctx, insn, op);
                return;
            }
            break;
        case 0x00007000:
            switch (op & FUNCT7_MASK) {
            case 0x00000000:
                trans_and(ctx, insn, op);
                return;
            case 0x02000000:
                trans_remu(ctx, insn, op);
                return;
            }
            break;
        }
        break;
    case 0x00000037:
        trans_lui(ctx, insn, op);
        return;
    case 0x0000003b:
        switch (op & FUNCT3_MASK) {
        case 0x00000000:
            switch (op & FUNCT7_MASK) {
            case 0x00000000:
                trans_addw(ctx, insn, op);
                return;
            case 0x40000000:
                trans_subw(ctx, insn, op);
                return;
            case 0x02000000:
                trans_mulw(ctx, insn, op);
                return;
            }
            break;
        case 0x00001000:
            switch (op & FUNCT7_MASK) {
            case 0x00000000:
                trans_sllw(ctx, insn, op);
                return;
            }
            break;
        case 0x00004000:
            switch (op & FUNCT7_MASK) {
            case 0x02000000:
                trans_divw(ctx, insn, op);
                return;
            }
            break;
        case 0x00005000:
            switch (op & FUNCT7_MASK) {
            case 0x00000000:
                trans_srlw(ctx, insn, op);
                return;
            case 0x40000000:
                trans_sraw(ctx, insn, op);
                return;
            case 0x02000000:
                trans_divuw(ctx, insn, op);
                return;
            }
            break;
        case 0x00006000:
            switch (op & FUNCT7_MASK) {
            case 0x02000000:
                trans_remw(ctx, insn, op);
                return;
            }
            break;
        case 0x00007000:
            switch (op & FUNCT7_MASK) {
            case 0x02000000:
                trans_remuw(ctx, insn, op);
                return;
            }
            break;
        }
        break;
    case 0x00000063:
        switch (op & FUNCT3_MASK) {
        case 0x00000000:
            trans_beq(ctx, insn, op);
            return;
        case 0x00001000:
            trans_bne(ctx, insn, op);
            return;
        case 0x00004000:
            trans_blt(ctx, insn, op);
            return;
        case 0x00005000:
            trans_bge(ctx, insn, op);
            return;
        case 0x00006000:
            trans_bltu(ctx, insn, op);
            return;
        case 0x00007000:
            trans_bgeu(ctx, insn, op);
            return;
        }
        break;
    case 0x00000067:
        switch (op & FUNCT3_MASK) {
        case 0x00000000:
            trans_jalr(ctx, insn, op);
            return;
        }
        break;
    case 0x0000006f:
        trans_jal(ctx, insn, op);
        return;
    case 0x00000073:
        switch (op & FUNCT3_MASK) {
        case 0x00000000:
            trans_ecall(ctx, insn, op);
            return;
        case 0x00001000:
            trans_csrrw(ctx, insn, op);
            return;
        case 0x00002000:
            trans_csrrs(ctx, insn, op);
            return;
        case 0x00003000:
            trans_csrrc(ctx, insn, op);
            return;
        case 0x00005000:
            trans_csrrwi(ctx, insn, op);
            return;
        case 0x00006000:
            trans_csrrsi(ctx, insn, op);
            return;
        case 0x00007000:
            trans_csrrci(ctx, insn, op);
            return;
        }
        break;;
    }
    trans_illegal(ctx, insn, op);
}

static inline int insn_len(uint32_t opcode)
{
    return (opcode & 0x3) == 3 ? 4 : 2;
}

static int decode(trans_t *ctx, trans_block_t *block)
{
    uint32_t opcode;
    int len;
    insn_t *insn = block->insn;
    while (1) {
        opcode = cpu_lduw(ctx->pc, 0);
        len = insn_len(opcode);
        switch (len) {
        case 2:
            decode_insn16(ctx, insn, opcode);
            break;
        case 4:
            decode_insn32(ctx, insn, opcode);
            break;
        default:
            assert_not_reached();
        }
        block->count++;
        if (ctx->exit) {
            break;
        }
        if (block->count >= TRANS_MAX) {
            ctx->link = 1;
            *ctx->pc_next[0] = ctx->pc + len;
            break;
        }
        ctx->pc += len;
        insn++;
    }
    return 0;
}

trans_block_t *translate(trans_block_t *block)
{
    trans_t ctx;
    trans_block_t *last;

    ctx.pc = block->pc;
    ctx.count = 0;
    ctx.link = 0;
    ctx.exit = 0;
    ctx.pc_next[0] = &block->jmp_next[0].pc;
    ctx.pc_next[1] = &block->jmp_next[1].pc;

    decode(&ctx, block);
    insn_buffer_ptr = insn_buffer_ptr + ROUND_UP(block->count * sizeof (insn_t), 64);

    last = ctx.link ? block : NULL;
    return last;
}

