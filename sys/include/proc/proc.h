#ifndef PROC_H_
#define PROC_H_

#include <lib/types.h>
#include <lib/asm.h>

typedef uint16_t pid_t;

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


typedef struct Process {
  pid_t pid;
  uintptr_t cr3;
  uintptr_t rsp_base;
  uintptr_t rsp;
  struct Process* next;
} process_t;


typedef struct {
  uint8_t lapic_id;
  process_t* queue_base;
  process_t* queue_head;
  size_t queue_size;
} core_t;


_noreturn void tasking_init(void);
uint8_t is_smp_supported(void);

#endif
