#ifndef PROC_H_
#define PROC_H_

#include <lib/types.h>
#include <lib/asm.h>

typedef uint16_t pid_t;


typedef struct {
  pid_t pid;

} process_t;


_noreturn void tasking_init(void);
uint8_t is_smp_supported(void);

#endif
