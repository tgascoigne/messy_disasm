#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_

#include <stdint.h>
#include <stdbool.h>

/* these need reordering.. */
#define OP_PUSH 1
#define OP_MOV 2
#define OP_AND 3
#define OP_SUB 4
#define OP_MOVL 5
#define OP_CALL 6
#define OP_LEAVE 7
#define OP_RET 8
#define OP_ADD 9
#define OP_TEST 10
#define OP_JZ 11
#define OP_POP 12
#define OP_MAX 0xFF

static char mnemonics[OP_MAX][16] = {
	[OP_PUSH] = "push",
	[OP_MOV] = "mov",
	[OP_AND] = "and",
	[OP_SUB] = "sub",
	[OP_MOVL] = "movl",
	[OP_CALL] = "call",
	[OP_LEAVE] = "leave",
	[OP_RET] = "ret",
	[OP_ADD] = "add",
	[OP_TEST] = "test",
	[OP_JZ] = "jz",
	[OP_POP] = "pop"
};

#define OPER_NONE 0
#define OPER_REG 1
#define OPER_ADDR 2
#define OPER_IMM 3
#define OPER_ABS_ADDR 4
#define OPER_RM 5
#define OPER_IMM8 6
#define OPER_IMM16 7
#define OPER_IMM32 8
#define OPER_REL_ADDR 9
#define OPER_REL8_ADDR 10

#define SZ_8 0
#define SZ_16 1
#define SZ_32 2

#define FLAG_MODRM (1 << 0) /* operation has modrm byte */
#define FLAG_EXTD_OPCODE (1 << 1) /* the reg field in modrm is part of opcode */

typedef int8_t reg_t;
typedef uint32_t imm_t;
typedef uint32_t abs_addr_t;

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
		abs_addr_t abs_addr;
	} op;
};

struct istr {
	uint32_t vaddr;
	istr_def_t* definition;
	uint16_t opcode;
	uint8_t operation;
	uint8_t op_size;
	uint8_t addr_size;
	operand_t src_oper;
	operand_t dst_oper;
};

static char reg_table[3][8][16] = {
	{ "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" },
	{ "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" },
	{ "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi" }
};

static istr_def_t istr_table[] = {
	{ .opcode = 0x50, .operation = OP_PUSH, .flags = 0, .src_oper = OPER_REG, .dst_oper = OPER_NONE },
	{ .opcode = 0x58, .operation = OP_POP, .flags = 0, .src_oper = OPER_REG, .dst_oper = OPER_NONE },
	{ .opcode = 0x74, .operation = OP_JZ, .flags = 0, .src_oper = OPER_REL8_ADDR, .dst_oper = OPER_NONE },
	{ .opcode = 0x81, .ex_opcode = 0x0, .operation = OP_ADD, .flags = FLAG_MODRM | FLAG_EXTD_OPCODE, .src_oper = OPER_IMM, .dst_oper = OPER_RM },
	{ .opcode = 0x83, .ex_opcode = 0x0, .operation = OP_ADD, .flags = FLAG_MODRM | FLAG_EXTD_OPCODE, .src_oper = OPER_IMM8, .dst_oper = OPER_RM },
	{ .opcode = 0x83, .ex_opcode = 0x4, .operation = OP_AND, .flags = FLAG_MODRM | FLAG_EXTD_OPCODE, .src_oper = OPER_IMM8, .dst_oper = OPER_RM },
	{ .opcode = 0x83, .ex_opcode = 0x5, .operation = OP_SUB, .flags = FLAG_MODRM | FLAG_EXTD_OPCODE, .src_oper = OPER_IMM8, .dst_oper = OPER_RM },
	{ .opcode = 0x85, .operation = OP_TEST, .flags = FLAG_MODRM, .src_oper = OPER_RM, .dst_oper = OPER_REG },
	{ .opcode = 0x89, .operation = OP_MOV, .flags = FLAG_MODRM, .src_oper = OPER_RM, .dst_oper = OPER_REG },
	{ .opcode = 0x8B, .operation = OP_MOV, .flags = FLAG_MODRM, .src_oper = OPER_RM, .dst_oper = OPER_REG },
	{ .opcode = 0xB8, .operation = OP_MOV, .flags = 0, .src_oper = OPER_IMM, .dst_oper = OPER_REG },
	{ .opcode = 0xC3, .operation = OP_RET, .flags = 0, .src_oper = OPER_NONE, .dst_oper = OPER_NONE },
	{ .opcode = 0xC7, .ex_opcode = 0x0, .operation = OP_MOVL, .flags = FLAG_MODRM | FLAG_EXTD_OPCODE, .src_oper = OPER_IMM, .dst_oper = OPER_RM },
	{ .opcode = 0xC9, .operation = OP_LEAVE, .flags = 0, .src_oper = OPER_NONE, .dst_oper = OPER_NONE },
	{ .opcode = 0xE8, .operation = OP_CALL, .flags = 0, .src_oper = OPER_REL_ADDR, .dst_oper = OPER_NONE },
	{ 0 }
};

void istr_to_string(istr_t* istr, char* out);
void op_to_string(istr_t* istr, operand_t* op, char* out);

#endif /* _INSTRUCTION_H_ */
