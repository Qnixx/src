.section .text

.global _start
_start:
  movq $0, %rbp
  pushq %rbp
  pushq %rbp
  movq %rsp, %rbp

  call __qlibc_init

  popq %rdi
  popq %rsi

  call main
  jmp .

.size _start, . - _start
