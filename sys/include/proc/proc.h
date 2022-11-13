#ifndef PROC_H_
#define PROC_H_

#include <lib/types.h>
#include <lib/asm.h>

typedef uint16_t pid_t;

typedef struct {
  pid_t pid;
} process_t;


typedef struct {
  size_t lapic_id;
  process_t* queue_head;
  process_t* queue_base;
  size_t queue_size;

  // For choosing a core 
  // to put a process on.
  uint8_t roll;
} core_t;


_noreturn void tasking_init(void);
extern core_t* proc_cores;

#endif
