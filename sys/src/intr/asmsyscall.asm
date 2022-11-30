bits 64
global syscall_entry

extern trap_entry

syscall_entry:
  cli
  push 0x80
  push rbp
  jmp trap_entry
