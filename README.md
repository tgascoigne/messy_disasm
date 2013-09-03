# Messy Disassembler #
A small, messy, partially implemented x86 disassembler.

After playing with the ELF object format, I wanted to write a disassembler. This is that disassembler.

It is not beautiful, elegant, fast, or even legible, but it was fun to write.

Currently it has definitions for around 20 opcodes, but I'm sure it could handle more if someone took the time to type them up.

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

./messy hello foo
8048430 <foo>:
 8048430: push %ebp
 8048431: mov %esp,%ebp
 8048433: sub $0x38,%esp
 8048436: cmp $0xc8,0x8(%ebp)
 804843d: jnz 8048449
 804843f: movl $0x64,0x8(%esp)
 8048446: call 804842b
 804844b: movl $0x1f4,-0x10(%ebp)
 8048452: mov 0x8(%ebp),%eax
 8048455: mov %eax,0x8(%esp)
 8048459: movl $0x8048550,0x4(%esp)
 8048461: lea -0x20(%ebp),%eax
 8048464: mov %eax,0x4(%esp)
 8048467: call 804831b
 804846c: shl 0x8(%ebp)
 804846f: mov -0x10(%ebp),%eax
 8048472: add %eax,0x8(%ebp)
 8048475: movl $0x0,-0xc(%ebp)
 804847c: jmp 8048486
 804847e: mov -0xc(%ebp),%eax
 8048481: sub %eax,0x8(%ebp)
 8048484: add $0x1,-0xc(%ebp)
 8048488: cmp $0x9,-0xc(%ebp)
 804848c: jle 804847c
 804848e: sub $0x2,0x8(%ebp)
 8048492: mov 0x8(%ebp),%eax
 8048495: leave 
 8048496: ret 
```