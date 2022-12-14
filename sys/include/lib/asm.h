#ifndef ASM_H_
#define ASM_H_


#define ASMV(_asm) __asm__ __volatile__(_asm)
#define _packed __attribute__((packed))
#define _isr __attribute__((interrupt))
#define _noreturn __attribute__((noreturn))
#define _naked __attribute__((naked))
#define _aligned(n) __attribute__((aligned(n)))
#define _unused __attribute__((unused))

#endif
