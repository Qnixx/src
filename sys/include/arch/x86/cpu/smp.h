#ifndef SMP_H_
#define SMP_H_

#include <lib/types.h>
#include <proc/proc.h>

// Returns 1 if CPU has only one core, otherwise 0.
uint8_t __smp_bootstrap_cores(core_t** cores);
uint8_t smp_get_core_count(void);


#endif
