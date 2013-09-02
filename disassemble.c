#include "disassemble.h"

#include <stdio.h>

#include "read_elf.h"
#include "decode.h"

/**
 * Disassembles a given function from an elf object file
 */
int disasm_func(elf_t* elf, char* func)
{
	/* gather all the info about the symbol we need */
	int symbol;
	uint64_t faddr;
	uint32_t vaddr;
	int ret = 0;

	ret = elf_get_symbol_by_name(elf, func, &symbol);
	if (ret != 0) {
		fprintf(stderr, "no such symbol '%s'\n", func);
		return DISASM_SYM_FAIL;
	}

	ret = elf_get_symbol_faddr(elf, symbol, &faddr);
	if (ret != 0) {
		fprintf(stderr, "unable to map symbol: %d\n", ret);
		return DISASM_SYM_FAIL;
	}

	ret = elf_get_symbol_vaddr(elf, symbol, &vaddr);
	if (ret != 0) {
		fprintf(stderr, "unable to map symbol: %d\n", ret);
		return DISASM_SYM_FAIL;
	}

	/* begin disassembly */
	unsigned char* ip = (unsigned char*)&elf->elf_data[faddr];
	uint32_t virt_ip = vaddr;
	istr_t instruction;
	char istr_str[16];
	bool done = false;

	printf("%x <%s>:\n", virt_ip, func);
	while (!done) {
		/* decode the next instruction */
		unsigned char* orig_ip = ip;
		ret = istr_decode(&ip, virt_ip, &instruction);
		if (ret != 0) {
			printf("unable to decode instruction at %x\n", virt_ip);
			return DISASM_DECODE_FAIL;
		}

		/* convert to a string and print the instruction */
		istr_to_string(&instruction, istr_str);
		printf("%s\n", istr_str);

		/* calculate the virtual address of the next instruction */
		virt_ip += (ip - orig_ip);

		/* check if the func has returned */
		if (instruction.operation == OP_RET) {
			done = true;
		}
	}

	return 0;
}










