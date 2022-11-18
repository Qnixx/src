#ifndef ASM_H_
#define ASM_H_

#include <lib/types.h>


#define ASMV(_asm) __asm__ __volatile__(_asm)
#define _packed __attribute__((packed))
#define _isr __attribute__((interrupt))
#define _noreturn __attribute__((noreturn))
#define _naked __attribute__((naked))

#define CLI_SLEEP                               \
  for (uint64_t i = 0; i < 100000000; ++i) {    \
    ASMV("cli");                                \
  }

#endif
