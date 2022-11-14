bits 64

global _irq0
extern trap_entry
extern lapic_send_eoi

%macro pre_eoi 0
  push rax
  push rbx
  push rcx
  push rdx
  push rsi
  push rdi
%endmacro

%macro post_eoi 0
  pop rdi
  pop rsi
  pop rdx
  pop rcx
  pop rbx
  pop rax
%endmacro

_irq0:
  cli
  pre_eoi
  call lapic_send_eoi
  post_eoi
  push 0x20
  push rbp
  jmp trap_entry
