#ifndef _DECODE_H_
#define _DECODE_H_

#include <stdint.h>
#include <stdbool.h>

#include "instruction.h"

typedef struct elf elf_t;

int istr_decode(elf_t* elf, unsigned char** _addr, uint32_t ip, istr_t* out);

#endif /* _DECODE_H_ */
