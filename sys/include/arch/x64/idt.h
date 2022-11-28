#ifndef IDT_H_
#define IDT_H_

#include <lib/asm.h>
#include <lib/types.h>


typedef struct {
  uint16_t isr_low16;
  uint16_t cs;
  uint8_t ist : 2;
  uint8_t zero : 4;
  uint8_t type : 4;
  uint8_t zero1 : 1;
  uint8_t dpl : 2;
  uint8_t p : 1;
  uint16_t isr_mid16;
  uint32_t isr_high32;
  uint32_t reserved;
} idt_gate_descriptor_t;


typedef struct {
  uint16_t limit;
  uint64_t base;
} _packed idtr_t;


void load_idt(void);

#endif
