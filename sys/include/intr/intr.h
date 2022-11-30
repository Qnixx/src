#ifndef INTR_H_
#define INTR_H_

#include <lib/types.h>

void intr_init(void);
__attribute__((naked)) void trap_exit(void);

#endif // INTR_H_
