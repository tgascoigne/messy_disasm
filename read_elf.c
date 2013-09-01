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

#define elf_assert(condition, error_ret)					\
	if (!(condition)) {										\
		fprintf(stderr, "assert failed: %s\n", #condition);	\
		__asm__("int3");									\
		return error_ret;									\
	}

static int elf_read_header(elf_t* elf);
static int elf_read_shdr(elf_t* elf);
static int elf_read_phdr(elf_t* elf);
static int elf_read_symtab(elf_t* elf);

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
	ret = elf_read_header(elf);	elf_assert(ret == 0, ret);
	ret = elf_read_shdr(elf);	elf_assert(ret == 0, ret);
	ret = elf_read_symtab(elf);	elf_assert(ret == 0, ret);	
	ret = elf_read_phdr(elf);	elf_assert(ret == 0, ret);

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
	int ret = 0;
	char sh_name[32];
	for (int i = 1; i < elf->num_sections; i++) {
		ret = elf_get_section_name(elf, i, sh_name);
		if (ret == 0) {
			printf("section %d: %s %x\n", i, sh_name, elf->shdr[i].sh_offset);
		} else {
			printf("section %d: error\n", i);
		}
	}
}

/**
 * Prints each segment and their virtual memory mapping
 */
void elf_print_segments(elf_t* elf)
{
	for (int i = 0; i < elf->num_segments; i++) {
		uint64_t end_addr = elf->phdr[i].p_vaddr + elf->phdr[i].p_memsz;
		printf("segment %2d: 0x%x-0x%x\n", i, elf->phdr[i].p_vaddr, end_addr);
	}
}

/**
 * Prints the name, type, vaddr, and file addr of each symbol
 */
void elf_print_symbols(elf_t* elf)
{
	int ret = 0;
	char sym_name[32];
	char sym_type[32];
	for (int i = 1; i < elf->num_symbols; i++) {
		ret = elf_get_symbol_name(elf, i, sym_name); elf_assert(ret == 0, ret);
		Elf32_Sym* symbol = &elf->symtab[i];

		/* fill in a blank name */
		if (strcmp(sym_name, "") == 0) {
			sprintf(sym_name, "no name");
		}

		/* convert the type to a string */
		switch (ELF64_ST_TYPE(symbol->st_info)) {
		case STT_NOTYPE:
			sprintf(sym_type, "no type");
			break;
		case STT_OBJECT:
			sprintf(sym_type, "object");
			break;
		case STT_FUNC:
			sprintf(sym_type, "function");
			break;
		case STT_SECTION:
			sprintf(sym_type, "section");
			break;
		case STT_FILE:
			sprintf(sym_type, "file");
			break;
		}

		/* find it's virtual and file addr */
		Elf32_Addr vaddr = symbol->st_value;
		uint64_t faddr = 0;
		elf_map_vaddr(elf, vaddr, &faddr); /* ignore mapping errors - dynamic linking etc. */
		
		if (ret == 0) {
			printf("symbol %3d: %-10s %-10s vaddr: %x faddr: %x\n", i, sym_type, sym_name, vaddr, faddr);
		} else {
			printf("symbol %d: error\n", i);
		}
	}
}

/**
 * Maps a virtual address to an offset in the elf file
 */
int elf_map_vaddr(elf_t* elf, Elf32_Addr vaddr, uint64_t* faddr)
{
	/* try to find the segment which contains the virtual address */
	for (int i = 1; i < elf->num_segments; i++) {
		Elf32_Phdr* segment = &elf->phdr[i];
		Elf32_Addr base_addr = segment->p_vaddr;
		Elf32_Addr lim_addr = segment->p_vaddr + elf->phdr[i].p_memsz;
		if (vaddr > base_addr && vaddr < lim_addr) {
			/* Success! */
			Elf32_Addr offset_addr = vaddr - base_addr;
			*faddr = segment->p_offset + offset_addr;
			return 0;
		}
	}
	return ELF_UNMAPPED;
}

/**
 * Maps a virtual address to a section
 */
int elf_map_vaddr_section(elf_t* elf, Elf32_Addr vaddr, int* out)
{
	/* try to find the section which contains the virtual address */
	for (int i = 1; i < elf->num_sections; i++) {
		Elf32_Shdr* section = &elf->shdr[i];
		Elf32_Addr base_addr = section->sh_addr;
		Elf32_Addr lim_addr = section->sh_addr + section->sh_size;
		if (vaddr > base_addr && vaddr < lim_addr) {
			/* Success! */
			*out = i;
			return 0;
		}
	}
	return ELF_UNMAPPED;
}

/**
 * Reads the elf header
 */
static int elf_read_header(elf_t* elf)
{
	elf->header = (Elf32_Ehdr*)elf->elf_data;
	elf_assert(elf->header->e_ident[EI_MAG0] == ELFMAG0, ELF_INVALID);
	elf_assert(elf->header->e_ident[EI_MAG1] == ELFMAG1, ELF_INVALID);
	elf_assert(elf->header->e_ident[EI_MAG2] == ELFMAG2, ELF_INVALID);
	elf_assert(elf->header->e_ident[EI_MAG3] == ELFMAG3, ELF_INVALID);
	elf_assert(elf->header->e_ident[EI_CLASS] == ELFCLASS32, ELF_INVALID_EXECUTABLE);
	elf_assert(elf->header->e_type == ET_EXEC, ELF_INVALID_EXECUTABLE);
	return 0;
}

/**
 * Reads the elf section headers
 */
static int elf_read_shdr(elf_t* elf)
{
	elf->shdr = (Elf32_Shdr*)(elf->elf_data + elf->header->e_shoff);
	elf->num_sections = elf->header->e_shnum;
	/* locate the common section indices */
	elf->sec_shstrtab = elf->header->e_shstrndx;
	int ret = 0;
	ret = elf_get_section_by_name(elf, ".strtab", &elf->sec_strtab);
	elf_assert(ret == 0, ret);
	return 0;
}

/**
 * Reads the elf program headers
 */
static int elf_read_phdr(elf_t* elf)
{
	elf->phdr = (Elf32_Phdr*)(elf->elf_data + elf->header->e_phoff);
	elf->num_segments = elf->header->e_phnum;
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
	elf->symtab = (Elf32_Sym*)(elf->elf_data + elf->shdr[symtab_sec].sh_offset);
	elf->num_symbols = elf->shdr[symtab_sec].sh_size/sizeof(Elf32_Sym);
	return 0;
}

/**
 * Locates a section's index by name
 */
int elf_get_section_by_name(elf_t* elf, char* name, int* out)
{
	char sec_name[32];
	int ret = 0;
	for (int i = 1; i < elf->num_sections; i++) {
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
int elf_get_section_name(elf_t* elf, int section, char* out)
{
	elf_assert(section < elf->num_sections, ELF_INVALID_SECTION);
	int strtab_ofs = elf->shdr[section].sh_name;
	int ret = elf_get_strtab_entry(elf, elf->sec_shstrtab, strtab_ofs, out);
	elf_assert(ret == 0, ret);
	return 0;
}

/**
 * Looks up the file address of a symbol
 */
int elf_get_symbol_faddr(elf_t* elf, int symidx, uint64_t* out)
{
	Elf32_Sym* symbol = &elf->symtab[symidx];
	Elf32_Addr vaddr = symbol->st_value;
	int ret = elf_map_vaddr(elf, vaddr, out); elf_assert(ret == 0, ELF_UNMAPPED);
	return 0;
}

/**
 * Looks up the section of a symbol
 */
int elf_get_symbol_section(elf_t* elf, int symidx, int* section)
{
	Elf32_Sym* symbol = &elf->symtab[symidx];
	Elf32_Addr vaddr = symbol->st_value;
	int ret = elf_map_vaddr_section(elf, vaddr, section); elf_assert(ret == 0, ELF_UNMAPPED);
	return 0;
}

/**
 * Looks up a symbol's index by name
 */
int elf_get_symbol_by_name(elf_t* elf, char* name, int* out)
{
	char sym_name[32];
	int ret = 0;
	for (int i = 0; i < elf->num_symbols; i++) {
		elf_get_symbol_name(elf, i, sym_name); /* ignore errors */
		if (strcmp(sym_name, name) == 0) {
			*out = i;
		}
	}
	return 0;
}

/**
 * Looks up a symbol's name in strtab by it's index
 */
int elf_get_symbol_name(elf_t* elf, int symbol, char* out)
{
	elf_assert(symbol < elf->num_symbols, ELF_INVALID_SYMBOL);
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
int elf_get_strtab_entry(elf_t* elf, int strtab, int ofs, char* out)
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
int elf_get_section_data(elf_t* elf, int section, char** out)
{
	elf_assert(section < elf->num_sections, ELF_INVALID_SECTION);
	uint64_t file_ofs = elf->shdr[section].sh_offset;
	*out = elf->elf_data+file_ofs;
	return 0;
}

/**
 * Gets the faddr of a section by its index
 */
int elf_get_section_faddr(elf_t* elf, int section, uint64_t* faddr)
{
	elf_assert(section < elf->num_sections, ELF_INVALID_SECTION);
	*faddr = elf->shdr[section].sh_offset;
	return 0;
}

/**
 * Gets the length of a section by its index
 */
int elf_get_section_len(elf_t* elf, int section, uint64_t* len)
{
	elf_assert(section < elf->num_sections, ELF_INVALID_SECTION);
	*len = elf->shdr[section].sh_size;
	return 0;	
}
