#ifndef _DISASSEMBLE_H_
#define _DISASSEMBLE_H_

#define DISASM_SYM_FAIL 1
#define DISASM_DECODE_FAIL 2

typedef struct elf elf_t;

int disasm_func(elf_t* elf, char* func);

#endif /* _DISASSEMBLE_H_ */
