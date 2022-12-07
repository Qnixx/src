#ifndef SCHED_H_
#define SCHED_H_

#include <lib/types.h>
#include <lib/asm.h>

typedef struct {
  uint64_t rdi;
  uint64_t rsi;
  uint64_t rbx;
  uint64_t rdx;
  uint64_t rcx;
  uint64_t rax;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  uint64_t rbp;
  uint64_t trapno;
  uint64_t rip;
  uint16_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint16_t ss;
} _packed trapframe_t;

void sched_init(void);

#endif
