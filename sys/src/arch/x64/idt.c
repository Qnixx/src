#include <arch/x64/idt.h>
#include <arch/x86/apic/ioapic.h>
#include <firmware/acpi/acpi.h>
#include <lib/module.h>

#define TRAP_GATE_FLAGS 0x8F
#define INT_GATE_FLAGS 0x8E
#define IDT_INT_GATE_USER 0xEE

MODULE_NAME("idt");
MODULE_DESCRIPTION("Interrupt descriptor table");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");

static idt_gate_descriptor_t idt[256];
static idtr_t idtr = {
  .limit = sizeof(idt_gate_descriptor_t) * 256 - 1,
  .base = (uint64_t)&idt
};


static void set_desc(uint8_t vector, void* isr, uint8_t flags) {
  uint64_t addr = (uint64_t)isr;
  idt_gate_descriptor_t* desc = &idt[vector];

  desc->isr_low16 = addr & 0xFFFF;
  desc->isr_mid16 = (addr >> 16) & 0xFFFF;
  desc->isr_high32 = (addr >> 32);
  desc->cs = 0x28;
  desc->ist = 0;
  desc->zero = 0;
  desc->zero1 = 0;
  desc->reserved = 0;
  desc->type = flags;
  desc->dpl = 3;
  desc->p = 1;
}


void load_idt(void) {
  ASMV("lidt %0" :: "m" (idtr));
}


void register_exception_handler(uint8_t vector, void* isr) {
  set_desc(vector, isr, TRAP_GATE_FLAGS);
}

void register_int_handler(uint8_t vector, void* isr) {
  set_desc(vector, isr, INT_GATE_FLAGS);
}

void register_irq_handler(uint8_t irq, void* isr) {
  ioapic_set_entry(acpi_remap_irq(irq), 0x20 + irq);
  register_int_handler(0x20 + irq, isr);
}
