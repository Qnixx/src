#include <proc/proc.h>


void trap(trapframe_t* tf) {
  switch (tf->trapno) {
    case 0x20:
      break;
  }
}
