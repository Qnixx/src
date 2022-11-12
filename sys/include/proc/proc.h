#ifndef PROC_H_
#define PROC_H_

#include <lib/types.h>
#include <lib/asm.h>


typedef struct {
  size_t lapic_id;
} core_t;


_noreturn void tasking_init(void);

#endif
