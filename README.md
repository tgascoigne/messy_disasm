# Messy Disassembler #
A small, messy, partially implemented x86 disassembler.

After playing with the ELF object format, I wanted to write a disassembler. This is that disassembler.

It is not beautiful, elegant, fast, or even legible, but it was fun to write.

Currently it has definitions for around 20 opcodes, but I'm sure it could handle more if someone took the time to type them up.

Sample output:
```
$ ./messy hello _init
80482b4 <_init>:
 80482b4: push %ebx
 80482b5: sub $0x8,%esp
 80482b8: call 8048360 <__x86.get_pc_thunk.bx+0>
 80482bd: add $0x14a3,%ebx
 80482c3: mov -0x4(%ebx),%eax
 80482c9: test %eax,%eax
 80482cb: jz 80482d2 <_init+1e>
 80482cd: call 8048300 <_init+4c>
 80482d2: add $0x8,%esp
 80482d5: pop %ebx
 80482d6: ret 

$ ./messy hello main 
8048497 <main>:
 8048497: push %ebp
 8048498: mov %esp,%ebp
 804849a: and $0xf0,%esp
 804849d: sub $0x10,%esp
 80484a0: movl $0x8048559,(%esp)
 80484a7: call 80482f0 <_init+3c>
 80484ac: movl $0xc8,(%esp)
 80484b3: call 8048430 <foo+0>
 80484b8: mov $0x0,%eax
 80484bd: leave 
 80484be: ret 

$ ./messy hello foo 
8048430 <foo>:
 8048430: push %ebp
 8048431: mov %esp,%ebp
 8048433: sub $0x38,%esp
 8048436: cmp $0xc8,0x8(%ebp)
 804843d: jnz 804844b <foo+1b>
 804843f: movl $0x64,0x8(%esp)
 8048446: call 8048430 <foo+0>
 804844b: movl $0x1f4,-0x10(%ebp)
 8048452: mov 0x8(%ebp),%eax
 8048455: mov %eax,0x8(%esp)
 8048459: movl $0x8048550,0x4(%esp)
 8048461: lea -0x20(%ebp),%eax
 8048464: mov %eax,0x4(%esp)
 8048467: call 8048320 <_init+6c>
 804846c: shl 0x8(%ebp)
 804846f: mov -0x10(%ebp),%eax
 8048472: add %eax,0x8(%ebp)
 8048475: movl $0x0,-0xc(%ebp)
 804847c: jmp 8048488 <foo+58>
 804847e: mov -0xc(%ebp),%eax
 8048481: sub %eax,0x8(%ebp)
 8048484: add $0x1,-0xc(%ebp)
 8048488: cmp $0x9,-0xc(%ebp)
 804848c: jle 804847e <foo+4e>
 804848e: sub $0x2,0x8(%ebp)
 8048492: mov 0x8(%ebp),%eax
 8048495: leave 
 8048496: ret  
```