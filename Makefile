CFLAGS = -std=gnu99 -g

OUT = messy
HEADERS = read_elf.h decode.h instruction.h disassemble.h
OBJECTS = messy.c read_elf.c decode.c instruction.c disassemble.c

all: $(OUT) hello hello.386

$(OUT): $(OBJECTS) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

hello: hello.c
	$(CC) $(CFLAGS) -m32 -o $@ $^

hello.386: hello
	objdump -d $< > $@

clean:
	-rm -f $(OUT) hello hello.386 *.o

.PHONY: all clean
