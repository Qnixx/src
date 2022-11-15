#include <proc/sched.h>


void trap(trapframe_t* tf) {
  switch (tf->trapno) {
    case 0x20:
      sched(tf);
      break;
  }
}
