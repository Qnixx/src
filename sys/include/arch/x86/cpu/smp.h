#ifndef SMP_H_
#define SMP_H_

#include <lib/types.h>

// Returns 1 if CPU has only one core, otherwise 0.
uint8_t __smp_bootstrap_cores(void);


#endif
