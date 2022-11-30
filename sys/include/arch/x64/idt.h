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
void register_exception_handler(uint8_t vector, void* isr);
void register_int_handler(uint8_t vector, void* isr);
void register_user_int_handler(uint8_t vector, void* isr);
void register_irq_handler(uint8_t irq, void* isr, size_t extra_redentry_data);


#endif
