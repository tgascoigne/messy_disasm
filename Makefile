CFLAGS = -std=gnu99 -g

all: elf hello

elf: elf.c read_elf.c
	$(CC) $(CFLAGS) -o $@ $^

hello: hello.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	-rm -f elf hello *.o
