#ifndef PROC_H_
#define PROC_H_

#include <lib/types.h>


typedef struct {
  uint8_t is_running : 1;
  uint32_t timeslice;
  uintptr_t cr3;
} thread_t;


#endif
