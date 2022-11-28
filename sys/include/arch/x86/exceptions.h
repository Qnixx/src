#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <lib/types.h>
#include <lib/asm.h>

#define E_ISR(vec) _isr static void __vec##vec(void* stackframe)
#define REF_E_ISR(vec) __vec##vec

void init_exceptions(void);


#endif
