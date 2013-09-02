#include "decode.h"

#include <stdio.h>
#include <string.h>

#define ISTR_NO_INSTRUCTION 1
#define ISTR_NO_OPERAND 2

#define OPERAND_SRC 0
#define OPERAND_DST 1

#define MODE_REG_INDIRECT 0
#define MODE_DISP8 1
#define MODE_DISP32 2
#define MODE_REG 3

#define OPCODE_BASE(i) (i & 0xF8)
#define OPCODE_REG(i) (i & 7)

#define MODRM_MODE(i) ((i >> 6) & 3)
#define MODRM_REG(i) ((i >> 3) & 7)
#define MODRM_OP(i) ((i >> 3) & 7)
#define MODRM_RM(i) (i & 7)

#define SIB_SS(i) ((i >> 6) & 3)
#define SIB_IDX(i) ((i >> 3) & 7)
#define SIB_BASE(i) (i & 7)

static void istr_decode_prefix(unsigned char** _addr, istr_t* out);
static int istr_decode_opcode(unsigned char** _addr, istr_t* istr_out);
static int istr_decode_operand(unsigned char* addr, unsigned char** istr_end, istr_t* istr, operand_t* operand, uint8_t direction);

/**
 * Decode an instruction located at addr with virtual address ip
 */
int istr_decode(unsigned char** _addr, uint32_t ip, istr_t* out)
{
	int ret = 0;
	unsigned char* addr = *_addr;
	unsigned char* istr_end;

	/* init an empty instruction */
	memset(out, 0, sizeof(operand_t));
	out->vaddr = ip;

	out->op_size = SZ_32;
	out->addr_size = SZ_32;

	/* decode it */
	istr_decode_prefix(&addr, out);
	ret = istr_decode_opcode(&addr, out);
	if (ret != 0) {
		return ISTR_NO_INSTRUCTION;
	}

	istr_end = addr;

	istr_decode_operand(addr, &istr_end, out, &out->src_oper, OPERAND_SRC);
	istr_decode_operand(addr, &istr_end, out, &out->dst_oper, OPERAND_DST);

	*_addr = istr_end;

	return 0;
}

/**
 * Decode any of the optional 4 byte prefixes
 */
static void istr_decode_prefix(unsigned char** _addr, istr_t* out)
{
	unsigned char* addr = *_addr;

	/* check for group 1 prefix: lock and repeat */
	switch (*addr) {
	case 0xF0: /* LOCK */
	case 0xF2: /* REPNE/REPNZ */
	case 0xF3: /* REP/REPE/REPZ */
		addr++;
		break;
	}

	/* check for group 2 prefix: segment overrides, branch hints (with Jcc instructions) */
	switch (*addr) {
	case 0x2E: /* CS segment override, Branch not taken hint */
	case 0x36: /* SS segment override, Branch taken hint */
	case 0x3E: /* DS segment override */
	case 0x26: /* ES segment override */
	case 0x64: /* FS segment override */
	case 0x65: /* GS segment override */
		addr++;
		break;
	}

	/* check for group 3 prefix: operand size override */
	if (*addr == 0x66) {
		out->op_size = SZ_16;
		addr++;
	}

	/* check for group 4 prefix: address size override */
	if (*addr == 0x67) {
		out->addr_size = SZ_16;
		addr++;
	}

	*_addr = addr;
}

/**
 * Decode an instruction opcode
 */
static int istr_decode_opcode(unsigned char** _addr, istr_t* istr_out)
{
	unsigned char* addr = *_addr;
	uint16_t opcode;
	uint8_t modrm;

	/* read the opcode */
	if (*addr == 0x0F) { /* extended 2-byte opcode */
		opcode = (0x0F << 8) + *(++addr);
	} else {
		opcode = *(addr++);
	}

	modrm = *addr;

	/* try to find it's definition */
	istr_def_t* def = &istr_table[0];
	while (def->opcode != 0) {
		bool found = false;
		/* test plain opcodes and extended opcodes */
		if (def->opcode == opcode) {
			found = true;
			if (def->flags & FLAG_EXTD_OPCODE && MODRM_OP(modrm) != def->ex_opcode) {
				found = false;
			}
		}

		/* test split opcodes */
		if (!(def->flags & FLAG_MODRM) && def->opcode == OPCODE_BASE(opcode)) {
			found = true;
		}

		if (found) {
			istr_out->definition = def;
			istr_out->operation = def->operation;
			istr_out->opcode = opcode;
			istr_out->src_oper.type = def->src_oper;
			istr_out->dst_oper.type = def->dst_oper;
			*_addr = addr;
			return 0;
		}

		def++;
	}

	return ISTR_NO_INSTRUCTION;
}

/**
 * Decode an instruction operand
 */
static int istr_decode_operand(unsigned char* addr, unsigned char** istr_end, istr_t* istr, operand_t* operand, uint8_t direction)
{
	uint8_t modrm;
	uint8_t sib;
	int8_t disp8;
	int32_t disp32;

	if (istr->definition->flags & FLAG_MODRM) {
		modrm = *(addr++);

		/* decode sib byte if necessary */
		if (MODRM_MODE(modrm) != MODE_REG && MODRM_RM(modrm) == 4) {
			sib = *(addr++);
		}

		/* decode displacement if necessary */
		if (MODRM_MODE(modrm) == MODE_DISP8) {
			disp8 = *((uint8_t*)addr);
			addr += 1;
		}
		if (MODRM_MODE(modrm) == MODE_DISP32 || (MODRM_MODE(modrm) == MODE_REG_INDIRECT && MODRM_RM(modrm) == 5)) {
			disp32 = *((uint32_t*)addr);
			addr += 4;
		}
	}

	/* ignore unnecessary operand */
	if (operand->type == OPER_NONE) {
		goto exit;
	}

	/* decode reg operands as part of opcode */
	if (operand->type == OPER_REG && !(istr->definition->flags & FLAG_MODRM)) {
		operand->op.reg = OPCODE_REG(istr->opcode);
		goto exit;
	}

	/* decode reg operands as part of modrm */
	if (operand->type == OPER_REG && istr->definition->flags & FLAG_MODRM) {
		operand->op.reg = MODRM_REG(modrm);
		goto exit;
	}

	/* decode rm operands */
	if (operand->type == OPER_RM && istr->definition->flags & FLAG_MODRM) {
		/* decode the SIB byte if necessary */
		if (MODRM_MODE(modrm) != MODE_REG && MODRM_RM(modrm) == 4) {
			operand->type = OPER_ADDR;
			operand->op.addr.idx = SIB_IDX(sib);
			if (operand->op.addr.idx == 4) { /* none */
				operand->op.addr.idx = -1;
			}
			operand->op.addr.base = SIB_BASE(sib);
			switch (MODRM_MODE(modrm)) {
			case MODE_REG_INDIRECT:
				if (MODRM_RM(modrm) == 5) {
					operand->op.addr.disp = disp32;
				}
				break;
			case MODE_DISP8:
				operand->op.addr.disp = disp8;
				break;
			case MODE_DISP32:
				operand->op.addr.disp = disp32;
				break;
			default:
				operand->op.addr.disp = 0;
				break;
			}
			switch (SIB_SS(sib)) {
			case 0: operand->op.addr.scale = 0; break;
			case 1: operand->op.addr.scale = 2; break;
			case 2: operand->op.addr.scale = 4; break;
			case 3: operand->op.addr.scale = 8; break;
			}
			goto exit;
		}

		switch (MODRM_MODE(modrm)) {
		case MODE_REG_INDIRECT:
			operand->type = OPER_ADDR;
			operand->op.addr.idx = -1;
			operand->op.addr.base = MODRM_RM(modrm);
			operand->op.addr.scale = 0;
			operand->op.addr.disp = 0;
			goto exit;
		case MODE_DISP8:
			operand->type = OPER_ADDR;
			operand->op.addr.idx = -1;
			operand->op.addr.base = MODRM_RM(modrm);
			operand->op.addr.scale = 0;
			operand->op.addr.disp = disp8;
			goto exit;
		case MODE_DISP32:
			operand->type = OPER_ADDR;
			operand->op.addr.idx = -1;
			operand->op.addr.base = MODRM_RM(modrm);
			operand->op.addr.scale = 0;
			operand->op.addr.disp = disp32;
			goto exit;
		case MODE_REG:
			operand->type = OPER_REG;
			operand->op.reg = MODRM_RM(modrm);
			goto exit;
		default:
			printf("unsupported addressing mode %d\n", MODRM_MODE(modrm));
		}
	}

	/* decode immediate operands */
	if (operand->type == OPER_IMM) {
		switch (istr->op_size) {
		case SZ_16:
			operand->type = OPER_IMM16;
			break;
		case SZ_32:
			operand->type = OPER_IMM32;
			break;
		}
	}

	if (operand->type == OPER_IMM8) {
		uint8_t* imm8 = (uint8_t*)addr;
		operand->type = OPER_IMM;
		operand->op.imm = *imm8;
		addr += 1;
		goto exit;
	}

	if (operand->type == OPER_IMM16) {
		uint16_t* imm16 = (uint16_t*)addr;
		operand->type = OPER_IMM;
		operand->op.imm = *imm16;
		addr += 2;
		goto exit;
	}

	if (operand->type == OPER_IMM32) {
		uint32_t* imm32 = (uint32_t*)addr;
		operand->type = OPER_IMM;
		operand->op.imm = *imm32;
		addr += 4;
		goto exit;
	}

	/* decode relative addresses */
	if (operand->type == OPER_REL_ADDR) {
		if (istr->op_size == SZ_16) {
			int16_t* rel16 = (int16_t*)addr;
			addr += 2;
			operand->type = OPER_ABS_ADDR;
			operand->op.abs_addr = (istr->vaddr + *rel16);
			goto exit;
		} else if (istr->op_size == SZ_32) {
			int32_t* rel32 = (int32_t*)addr;
			addr += 4;
			operand->type = OPER_ABS_ADDR;
			operand->op.abs_addr = (istr->vaddr + *rel32);
			goto exit;
		}
	}

exit:
	if (addr > *istr_end) {
		*istr_end = addr;
	}

	return 0;
}
