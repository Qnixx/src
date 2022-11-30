.global syscall

syscall:
  movq %rdi, (rdi)
  movq %rsi, (rsi)
  movq %rdx, (rdx)
  movq %rcx, (rcx)
  movq %r8, (r8)
  movq %r9, (r9)
  
  # TODO: Allow more registers.
  mov (rdi), %rax
  mov (rsi), %rbx
  mov (rdx), %rcx
  mov (rcx), %rdx
  int $0x80
  retq


.data
rdi: .quad 0
rsi: .quad 0
rdx: .quad 0
rcx: .quad 0
r8: .quad 0
r9: .quad 0
