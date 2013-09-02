CFLAGS = -std=gnu99 -g

all: elf hello hello.386

elf: elf.c read_elf.c decode.c instruction.c
	$(CC) $(CFLAGS) -o $@ $^

hello: hello.c
	$(CC) $(CFLAGS) -m32 -o $@ $^

hello.386: hello
	objdump -d $< > $@

clean:
	-rm -f elf hello *.o
