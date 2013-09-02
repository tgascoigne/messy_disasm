#ifndef _DECODE_H_
#define _DECODE_H_

#include <stdint.h>
#include <stdbool.h>

#include "instruction.h"

int istr_decode(unsigned char** addr, istr_t* out);

#endif /* _DECODE_H_ */
