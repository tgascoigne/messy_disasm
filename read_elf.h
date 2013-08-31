#ifndef _READ_ELF_H_
#define _READ_ELF_H_

#include <linux/elf.h>
#include <stdlib.h>

#define ELF_NO_FILE 1
#define ELF_INVALID 2
#define ELF_INVALID_EXECUTABLE 3
#define ELF_INVALID_SECTION 4
#define ELF_NO_SHSTRTAB 5
#define ELF_INVALID_SYMBOL 6

typedef struct elf elf_t;
struct elf {
	char* elf_data;
	size_t elf_length;
	Elf64_Ehdr* header;
	Elf64_Shdr* shdr;
	Elf64_Phdr* phdr;
	int symtab_length;
	Elf64_Sym* symtab;
	/* section indexes */
	int sec_strtab;
	int sec_shstrtab;
};

int elf_read(const char* elf_file, elf_t* elf);
void elf_free(elf_t* elf);
void elf_print_sections(elf_t* elf);
void elf_print_symbols(elf_t* elf);
void elf_print_segments(elf_t* elf);

#endif /* _READ_ELF_H_ */
