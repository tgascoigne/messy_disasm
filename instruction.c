#include "instruction.h"

#include <string.h>
#include <stdio.h>

/**
 * Produce a string representation of a given instruction
 */
void istr_to_string(istr_t* istr, char* out)
{
	char src_oper[64];
	char dst_oper[64];
	op_to_string(istr, &istr->src_oper, src_oper);
	op_to_string(istr, &istr->dst_oper, dst_oper);

	sprintf(out, "%8x: ", istr->vaddr);
	strcat(out, mnemonics[istr->operation]);
	strcat(out, " ");
	if (strcmp(src_oper, "") != 0) {
		strcat(out, src_oper);
	}

	if (strcmp(dst_oper, "") != 0) {
		strcat(out, ",");
		strcat(out, dst_oper);
	}
}

/**
 * Produce a string representation of a given operand
 */
void op_to_string(istr_t* istr, operand_t* op, char* out)
{
	reg_t reg_op = op->op.reg;
	imm_t imm_op = op->op.imm;
	addr_op_t* addr_op = &(op->op.addr);
	abs_addr_t abs_addr_op = op->op.abs_addr;
	char tmp[64];
	sprintf(out, "");
	switch (op->type) {
	case OPER_REG:
		sprintf(out, "%%%s", reg_table[istr->op_size][reg_op]);
		break;
	case OPER_IMM:
		sprintf(out, "$0x%x", imm_op);
		break;
	case OPER_ABS_ADDR:
		if (strcmp(abs_addr_op.sym, "") != 0) {
			sprintf(tmp, " <%s+%x>", abs_addr_op.sym, abs_addr_op.sym_ofs);
		} else {
			sprintf(tmp, "");
		}
		sprintf(out, "%x%s", abs_addr_op.addr, tmp);
		break;
	case OPER_ADDR:
		/* append the displacement */
		if (addr_op->disp != 0) { 
			int32_t d = addr_op->disp;
			sprintf(tmp, "%s0x%x", (d < 0 ? "-" : ""), (d < 0 ? -d : d));
			strcat(out, tmp);
		}

		strcat(out, "(");

		/* append the base */
		if (addr_op->base != -1) {
			sprintf(tmp, "%%%s", reg_table[istr->addr_size][addr_op->base]);
			strcat(out, tmp);
		}

		/* append the index + scale */
		if (addr_op->idx != -1) {
			strcat(out, ",");
			sprintf(tmp, "%%%s", reg_table[istr->addr_size][addr_op->idx]);
			strcat(out, tmp);
			strcat(out, ",");
			sprintf(tmp, "%d", addr_op->scale);
			strcat(out, tmp);
		}

		strcat(out, ")");
		break;
	default:
		sprintf(out, "");
		break;
	}
}
