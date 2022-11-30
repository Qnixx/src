#ifndef PROC_H_
#define PROC_H_

#include <lib/types.h>
#include <lib/asm.h>

typedef uint16_t pid_t;


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
