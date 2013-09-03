#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static uint32_t SOME_GLOBAL = 0xE1F;

int foo(int i) {
	if (i == 200) {
		foo(100);
	}
	char tmp[16];
	int x = 500;
	sprintf(tmp, "test %i\n", i);
	i *= 2;
	i += x;
	for (int y = 0; y < 10; y++) {
		i -= y;
	}
	i -= 2;
	return i;
}

int main(int argc, char** argv) {
	printf("Hello elf!\n");
	foo(200);
	return EXIT_SUCCESS;
}
