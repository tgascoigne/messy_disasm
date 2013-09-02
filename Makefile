CFLAGS = -std=gnu99 -g

HEADERS = read_elf.h decode.h instruction.h disassemble.h
OBJECTS = elf.c read_elf.c decode.c instruction.c disassemble.c

all: elf hello hello.386

elf: $(OBJECTS) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

hello: hello.c
	$(CC) $(CFLAGS) -m32 -o $@ $^

hello.386: hello
	objdump -d $< > $@

clean:
	-rm -f elf hello *.o
