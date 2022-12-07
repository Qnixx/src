#ifndef IDT_H_
#define IDT_H_

#include <lib/types.h>
#include <lib/asm.h>


struct interrupt_gate_descriptor {
  uint16_t offlow16;
  uint16_t cs;
  uint8_t ist : 2;
  uint8_t zero : 4;
  uint8_t attr : 4;
  uint8_t zero1 : 1;
  uint8_t dpl : 2;
  uint8_t p : 1;
  uint16_t offmid16;
  uint32_t offhigh32;
  uint32_t reserved;
};


struct idtr {
  uint16_t limit;
  uint64_t base;
} _packed;

void register_exception_handler(uint8_t vector, void* isr);
void register_irq(uint8_t irq, void* isr);
void load_idt(void);

#endif
