#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_

#include <stdint.h>
#include <stdbool.h>

#define OP_PUSH 1
#define OP_MOV 2
#define OP_AND 3
#define OP_SUB 4
#define OP_MOVL 5
#define OP_MAX 0xFF

#define OPER_NONE 0
#define OPER_REG 1
#define OPER_ADDR 2
#define OPER_IMM 3
#define OPER_RM 4
#define OPER_IMM8 5
#define OPER_IMM16 6
#define OPER_IMM32 7

#define SZ_8 0
#define SZ_16 1
#define SZ_32 2

#define FLAG_MODRM (1 << 0) /* operation has modrm byte */
#define FLAG_EXTD_OPCODE (1 << 1) /* the reg field in modrm is part of opcode */

typedef int8_t reg_t;
typedef uint32_t imm_t;

typedef struct istr_def istr_def_t;
typedef struct istr istr_t;
typedef struct operand operand_t;

typedef struct imm_op imm_op_t;
typedef struct addr_op addr_op_t;

struct istr_def {
	uint16_t opcode;
	uint8_t ex_opcode;
	uint8_t operation;
	uint8_t flags;
	uint8_t src_oper;
	uint8_t dst_oper;
};

struct addr_op {
	reg_t base;
	reg_t idx;
	uint8_t scale;
	int32_t disp;
};

struct operand {
	uint8_t type;
	union {
		reg_t reg;
		addr_op_t addr;
		imm_t imm;
	} op;
};

struct istr {
	istr_def_t* definition;
	uint16_t opcode;
	uint8_t operation;
	uint8_t op_size;
	uint8_t addr_size;
	operand_t src_oper;
	operand_t dst_oper;
};

void istr_to_string(istr_t* istr, char* out);
void op_to_string(istr_t* istr, operand_t* op, char* out);

#endif /* _INSTRUCTION_H_ */
