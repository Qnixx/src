#include <sched/sched.h>

void trap(trapframe_t* tf) {
  switch (tf->trapno) {
    default:
      break;
  }
}
