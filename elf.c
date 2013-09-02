#include "read_elf.h"
#include "decode.h"

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

	/* disassemble main */
	int symbol;
	uint64_t faddr;
	ret = elf_get_symbol_by_name(&elf, "main", &symbol);
	if (ret != 0) {
		fprintf(stderr, "no such symbol 'main'\n");
		return EXIT_FAILURE;
	}

	ret = elf_get_symbol_faddr(&elf, symbol, &faddr);
	if (ret != 0) {
		fprintf(stderr, "unable to map symbol: %d\n", ret);
		return EXIT_FAILURE;
	}

	unsigned char* ip = (unsigned char*)&elf.elf_data[faddr];
	istr_t instruction;

	char istr_str[16];
	for (int i = 0; i < 5; i++) {
		ret = istr_decode(&ip, &instruction);
		if (ret != 0) {
			printf("unable to decode instruction at %x\n", ip);
			ret = EXIT_FAILURE;
			goto exit;
		}
		istr_to_string(&instruction, istr_str);
		printf("%s\n", istr_str);
	}

	ret = EXIT_SUCCESS;
exit:
	elf_free(&elf);
	
	return ret;
}




