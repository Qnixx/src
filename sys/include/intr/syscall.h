#ifndef SYSCALL_H_
#define SYSCALL_H_

#include <proc/proc.h>

#define MAX_SYSCALLS 2

extern void(*syscalls[MAX_SYSCALLS])(trapframe_t* tf);

#endif // SYSCALL_H_
