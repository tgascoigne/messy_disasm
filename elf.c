#include "read_elf.h"
#include <stdio.h>

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "no input elf specified\n");
		return EXIT_FAILURE;
	}

	elf_t elf;
	int ret = elf_read(argv[1], &elf);
	if (ret != 0) {
		fprintf(stderr, "unable to parse elf: %d\n", ret);
		return EXIT_FAILURE;
	}

	elf_print_sections(&elf);
	elf_print_symbols(&elf);

	int symbol;
	uint64_t faddr;
	uint32_t* val;
	ret = elf_get_symbol_by_name(&elf, "SOME_GLOBAL", &symbol);
	ret = elf_get_symbol_faddr(&elf, symbol, &faddr);
	val = (uint32_t*)(elf.elf_data + faddr);
	printf("Value of SOME_GLOBAL: 0x%x\n", *val);

	elf_free(&elf);
	
	return EXIT_SUCCESS;
}
