#include "instruction.h"

#include <string.h>
#include <stdio.h>

void op_to_string(istr_t* istr, operand_t* op, char* out)
{
	reg_t r_op = op->op.reg;
	imm_t i_op = op->op.imm;
	addr_op_t* a_op = &(op->op.addr);
	char tmp[16];
	sprintf(out, "");
	switch (op->type) {
	case OPER_REG:
		sprintf(out, "%%%s", reg_table[istr->op_size][r_op]);
		break;
	case OPER_IMM:
		sprintf(out, "$0x%x", i_op);
		break;
	case OPER_ADDR:
		/* append the displacement */
		if (a_op->disp != 0) { 
			sprintf(tmp, "%d", a_op->disp);
			strcat(out, tmp);
		}

		strcat(out, "(");

		/* append the base */
		if (a_op->base != -1) {
			sprintf(tmp, "%%%s", reg_table[istr->addr_size][a_op->base]);
			strcat(out, tmp);
		}

		/* append the index + scale */
		if (a_op->idx != -1) {
			strcat(out, ",");
			sprintf(tmp, "%%%s", reg_table[istr->addr_size][a_op->idx]);
			strcat(out, tmp);
			strcat(out, ",");
			sprintf(tmp, "%d", a_op->scale);
			strcat(out, tmp);
		}

		strcat(out, ")");
		break;
	default:
		sprintf(out, "");
		break;
	}
}

void istr_to_string(istr_t* istr, char* out)
{
	char src_oper[16];
	char dst_oper[16];
	op_to_string(istr, &istr->src_oper, src_oper);
	op_to_string(istr, &istr->dst_oper, dst_oper);

	sprintf(out, "");
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
