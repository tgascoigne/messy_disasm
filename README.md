# Messy Disassembler #
A small, messy, partially implemented x86 disassembler.

After playing with the ELF object format, I wanted to write a disassembler. This is that disassembler.

It is not beautiful, elegant, fast, or even legible, but it was fun to write.

Currently it can disassemble just enough instructions to be able to handle the main function of a hello world compiled by GCC.
Sample output:
```
$ ./messy hello
8048400 <main>:
 8048400: push %ebp
 8048401: mov %ebp,%esp
 8048403: and $0xf0,%esp
 8048406: sub $0x10,%esp
 8048409: movl $0x80484b0,(%esp)
 8048410: call 80482cb
 8048415: mov $0x0,%eax
 804841a: leave 
 804841b: ret 
```