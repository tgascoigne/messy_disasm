#ifndef _READ_ELF_H_
#define _READ_ELF_H_

#include <linux/elf.h>
#include <stdlib.h>
#include <stdint.h>

#define ELF_NO_FILE 1
#define ELF_INVALID 2
#define ELF_INVALID_EXECUTABLE 3
#define ELF_INVALID_SECTION 4
#define ELF_NO_SHSTRTAB 5
#define ELF_INVALID_SYMBOL 6
#define ELF_UNMAPPED 7

typedef struct elf elf_t;
struct elf {
	char* elf_data;
	size_t elf_length;
	Elf32_Ehdr* header;
	Elf32_Shdr* shdr;
	Elf32_Phdr* phdr;
	Elf32_Sym* symtab;
	int num_segments;
	int num_sections;
	int num_symbols;
	/* section indexes */
	int sec_strtab;
	int sec_shstrtab;
};

int elf_read(const char* elf_file, elf_t* elf);
void elf_free(elf_t* elf);

void elf_print_sections(elf_t* elf);
void elf_print_symbols(elf_t* elf);
void elf_print_segments(elf_t* elf);

int elf_get_strtab_entry(elf_t* elf, int strtab, int ofs, char* out);

int elf_get_section_by_name(elf_t* elf, char* name, int* out);
int elf_get_section_name(elf_t* elf, int section, char* out);
int elf_get_section_data(elf_t* elf, int section, char** out);
int elf_get_section_faddr(elf_t* elf, int section, uint64_t* faddr);
int elf_get_section_len(elf_t* elf, int section, uint64_t* len);

int elf_get_symbol_faddr(elf_t* elf, int symbol, uint64_t* out);
int elf_get_symbol_vaddr(elf_t* elf, int symbol, uint32_t* out);
int elf_get_symbol_section(elf_t* elf, int symidx, int* section);
int elf_get_symbol_by_name(elf_t* elf, char* name, int* out);
int elf_get_symbol_name(elf_t* elf, int symbol, char* out);

int elf_map_vaddr(elf_t* elf, Elf32_Addr vaddr, uint64_t* faddr);
int elf_map_vaddr_section(elf_t* elf, Elf32_Addr vaddr, int* section);

#endif /* _READ_ELF_H_ */
