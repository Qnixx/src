bits 64

global trap_entry
global trap_new_task_entry

extern trap

trap_entry:
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rax
  push rcx
  push rdx
  push rbx
  push rsi
  push rdi

  mov rdi, rsp
  call trap

trap_exit:
  pop rdi
  pop rsi
  pop rbx
  pop rdx
  pop rcx
  pop rax
  pop r8
  pop r9
  pop r10
  pop r11
  pop r12
  pop r13
  pop r14
  pop r15
  pop rbp
  add rsp, 8
  iretq

;; RDI = Stack pointer.
trap_new_task_entry:
  or rdi, rdi
  jz .invalid

  mov rsp, rdi
  jmp trap_exit

  .invalid:
    retq
