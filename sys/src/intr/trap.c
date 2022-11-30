#include <proc/proc.h>
#include <intr/syscall.h>

void trap(trapframe_t* tf) {
  switch (tf->trapno) {
    case 0x20:
      break;
    case 0x80:
      if (tf->rax < 0 || tf->rax >= MAX_SYSCALLS) {
        break;
      }

      syscalls[tf->rax](tf);
  }
}
