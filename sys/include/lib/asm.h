#ifndef ASM_H_
#define ASM_H_

#define _noreturn __attribute__((noreturn))
#define _packed __attribute__((packed))
#define _isr __attribute__((interrupt))
#define ASMV(_asm) __asm__ __volatile__(_asm)

#define CLI_SLEEP \
  for (unsigned long long i = 0; i < 1000000; ++i) {          \
    ASMV("cli");                                              \
  }

#endif
