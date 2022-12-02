/*
 * Description: Interrupt descriptor table module.
 * Author(s): Ian Marco Moffett.
 *
 */

#include <arch/x86_64/idt.h>

#if defined(__x86_64__)

#define TRAP_GATE_FLAGS 0x8F
#define INT_GATE_FLAGS 0x8E
#define IDT_INT_GATE_USER 0xEE

struct interrupt_gate_descriptor idt[256];
struct idtr idtr = {
  .limit = sizeof(struct interrupt_gate_descriptor) * 256 - 1,
  .base = (uint64_t)&idt
};

static void set_desc(uint8_t vector, void* isr, uint8_t flags) {
  uint64_t addr = (uint64_t)isr;
  struct interrupt_gate_descriptor* desc = &idt[vector];

  desc->offlow16 = addr & 0xFFFF;
  desc->offmid16 = (addr >> 16) & 0xFFFF;
  desc->offhigh32 = (addr >> 32);
  desc->cs = 0x28;
  desc->ist = 0;
  desc->zero = 0;
  desc->zero1 = 0;
  desc->reserved = 0;
  desc->attr = flags;
  desc->dpl = 3;
  desc->p = 1;
}

void register_exception_handler(uint8_t vector, void* isr) {
  set_desc(vector, isr, TRAP_GATE_FLAGS);
}


void load_idt(void) {
  ASMV("lidt %0" :: "m" (idtr));
}

#endif  // defined(__x86_64__)
