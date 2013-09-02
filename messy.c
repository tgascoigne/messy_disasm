#include "read_elf.h"
#include "decode.h"
#include "disassemble.h"

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
	ret = disasm_func(&elf, "main");
	if (ret != 0) {
		fprintf(stderr, "unable to disassemble main: %d\n", ret);
		goto exit;
	}

	ret = EXIT_SUCCESS;
exit:
	elf_free(&elf);
	
	return ret;
}




