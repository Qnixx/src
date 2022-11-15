#ifndef PROC_H_
#define PROC_H_

#include <arch/x86/gdt.h>
#include <lib/types.h>
#include <lib/asm.h>

#define PSTACK_SIZE 0x1000

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
  trapframe_t tf; 
  uint64_t rsp;
  uint64_t stack_base;
  uint64_t cr3;
  uint8_t is_ring3;
  struct Process* next;
} process_t;


typedef struct {
  size_t lapic_id;
  process_t* queue_head;          // First process in queue.
  process_t* queue_base;          // Last process in queue.
  process_t* running_process;
  size_t queue_size;

  // Descriptor tables.
  struct gdt_descriptor* gdt;
  struct gdtr* gdtr;

  // For choosing a core 
  // to put a process on.
  uint8_t roll;
} core_t;


_noreturn void tasking_init(void);
uint64_t get_rip(void);
void proc_clone(uint64_t to_rip);

core_t* proc_find_core(size_t lapic_id);

extern core_t* proc_cores;

#endif
