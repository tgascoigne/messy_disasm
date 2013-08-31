/**
 * read_elf.c
 *
 * Reads some stuff from an ELF executable
 */
#include "read_elf.h"

#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define elf_assert(condition, error_ret)					\
	if (!(condition)) {										\
		fprintf(stderr, "assert failed: %s\n", #condition);	\
		__asm__("int3");									\
		return error_ret;									\
	}

static int elf_read_header(elf_t* elf);
static int elf_read_shdr(elf_t* elf);
static int elf_read_symtab(elf_t* elf);
static int elf_get_strtab_entry(elf_t* elf, int strtab, int ofs, char* out);
static int elf_get_section_by_name(elf_t* elf, char* name, int* out);
static int elf_get_section_name(elf_t* elf, int section, char* out);
static int elf_get_section_data(elf_t* elf, int section, char** out);
static int elf_get_symbol_name(elf_t* elf, int symbol, char* out);

/**
 * Reads an elf executable
 */
int elf_read(const char* elf_file, elf_t* elf) {
	/* get the elf file size */
	struct stat fstat;
	int ret;
	ret = stat(elf_file, &fstat);
	elf_assert(ret == 0, ELF_NO_FILE);

	/* allocate space for it */
	elf->elf_data = (char*)malloc(fstat.st_size);
	elf->elf_length = fstat.st_size;

	/* read it from disk */
	FILE* elf_fd = fopen(elf_file, "r");
	elf_assert(elf_fd != 0, ELF_NO_FILE);
	ret = fread(elf->elf_data, 1, elf->elf_length, elf_fd);
	elf_assert(ret == elf->elf_length, ELF_NO_FILE);
	fclose(elf_fd);

	/* parse it */
	ret = elf_read_header(elf);
	elf_assert(ret == 0, ret);
	ret = elf_read_shdr(elf);
	elf_assert(ret == 0, ret);
	ret = elf_read_symtab(elf);
	elf_assert(ret == 0, ret);	

	return EXIT_SUCCESS;
}

/**
 * Frees up the internal structures in an elf_t
 */
void elf_free(elf_t* elf)
{
	free(elf->elf_data);
}

/**
 * Prints the name of each section
 */
void elf_print_sections(elf_t* elf)
{
	/* print out the section names */
	int ret = 0;
	char sh_name[32];
	for (int i = 1; i < elf->header->e_shnum; i++) {
		ret = elf_get_section_name(elf, i, sh_name);
		if (ret == 0) {
			printf("section %d: %s\n", i, sh_name);
		} else {
			printf("section %d: error\n", i);
		}
	}
}

/**
 * Prints the name of each symbol
 */
void elf_print_symbols(elf_t* elf)
{
	/* print out the symbol table */
	int ret = 0;
	char sym_name[32];
	for (int i = 1; i < elf->symtab_length; i++) {
		ret = elf_get_symbol_name(elf, i, sym_name);
		if (ret == 0) {
			printf("symbol %d: %s\n", i, sym_name);
		} else {
			printf("symbol %d: error\n", i);
		}
	}
}

/**
 * Reads the elf header
 */
static int elf_read_header(elf_t* elf)
{
	elf->header = (Elf64_Ehdr*)elf->elf_data;
	elf_assert(elf->header->e_ident[EI_MAG0] == ELFMAG0, ELF_INVALID);
	elf_assert(elf->header->e_ident[EI_MAG1] == ELFMAG1, ELF_INVALID);
	elf_assert(elf->header->e_ident[EI_MAG2] == ELFMAG2, ELF_INVALID);
	elf_assert(elf->header->e_ident[EI_MAG3] == ELFMAG3, ELF_INVALID);
	elf_assert(elf->header->e_type == ET_EXEC, ELF_INVALID_EXECUTABLE);
	return 0;
}

/**
 * Reads the elf section headers
 */
static int elf_read_shdr(elf_t* elf)
{
	elf->shdr = (Elf64_Shdr*)(elf->elf_data + elf->header->e_shoff);
	/* locate the common section indices */
	elf->sec_shstrtab = elf->header->e_shstrndx;
	int ret = 0;
	ret = elf_get_section_by_name(elf, ".strtab", &elf->sec_strtab);
	elf_assert(ret == 0, ret);
	return 0;
}

/**
 * Locates the elf symtab
 */
static int elf_read_symtab(elf_t* elf)
{
	int symtab_sec;
	int ret = 0;
	ret = elf_get_section_by_name(elf, ".symtab", &symtab_sec);
	elf_assert(ret == 0, ret);
	elf->symtab = (Elf64_Sym*)(elf->elf_data + elf->shdr[symtab_sec].sh_offset);
	elf->symtab_length = elf->shdr[symtab_sec].sh_size/sizeof(Elf64_Sym);
	return 0;
}

/**
 * Locates a section's index by name
 */
static int elf_get_section_by_name(elf_t* elf, char* name, int* out)
{
	char sec_name[32];
	int ret = 0;
	for (int i = 1; i < elf->header->e_shnum; i++) {
		ret = elf_get_section_name(elf, i, sec_name);
		elf_assert(ret == 0, ret);
		if (strcmp(sec_name, name) == 0) {
			*out = i;
		}
	}
	return 0;
}

/**
 * Looks up a section's name in shstrtab by it's index
 */
static int elf_get_section_name(elf_t* elf, int section, char* out)
{
	elf_assert(section < elf->header->e_shnum, ELF_INVALID_SECTION);
	int strtab_ofs = elf->shdr[section].sh_name;
	int ret = elf_get_strtab_entry(elf, elf->sec_shstrtab, strtab_ofs, out);
	elf_assert(ret == 0, ret);
	return 0;
}

/**
 * Looks up a symbol's name in strtab by it's index
 */
static int elf_get_symbol_name(elf_t* elf, int symbol, char* out)
{
	elf_assert(symbol < elf->symtab_length, ELF_INVALID_SYMBOL);
	int strtab_ofs = elf->symtab[symbol].st_name;
	int ret = elf_get_strtab_entry(elf, elf->sec_strtab, strtab_ofs, out);
	elf_assert(ret == 0, ret);
	return 0;
}

/**
 * Reads a strtab entry
 *  - strtab: The section index of the strtab to read
 *  - ofs: The offset to read into the strtab
 */
static int elf_get_strtab_entry(elf_t* elf, int strtab, int ofs, char* out)
{
	char* shstrtab;
	int ret = elf_get_section_data(elf, strtab, &shstrtab);
	elf_assert(ret == 0, ELF_NO_SHSTRTAB);

	strcpy(out, shstrtab+ofs);
	return 0;
}

/**
 * Gets the contents of a section by its index
 */
static int elf_get_section_data(elf_t* elf, int section, char** out)
{
	elf_assert(section < elf->header->e_shnum, ELF_INVALID_SECTION);
	uint64_t file_ofs = elf->shdr[section].sh_offset;
	*out = elf->elf_data+file_ofs;
	return 0;
}
