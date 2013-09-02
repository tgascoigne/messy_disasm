# Messy Disassembler #
A small, messy, partially implemented x86 disassembler.

After playing with the ELF object format, I wanted to write a disassembler. This is that disassembler.

It is not beautiful, elegant, fast, or even legible, but it was fun to write.

Currently it can disassemble just enough instructions to be able to handle the main and _init functions of a hello world compiled by GCC.
Sample output:
```
$ ./messy hello main
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

$ ./messy hello _init  
8048290 <_init>:
 8048290: push %ebx
 8048291: sub $0x8,%esp
 8048294: call 804832b
 8048299: add $0x13f7,%ebx
 804829f: mov -4(%ebx),%eax
 80482a5: test %eax,%eax
 80482a7: jz 80482ac
 80482a9: call 80482db
 80482ae: add $0x8,%esp
 80482b1: pop %ebx
 80482b2: ret
```