#include <arch/x64/idt.h>
#include <lib/module.h>

MODULE_NAME("idt");
MODULE_DESCRIPTION("Interrupt descriptor table");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");

static idt_gate_descriptor_t idt[256];
static idtr_t idtr = {
  .limit = sizeof(idt_gate_descriptor_t) * 256 - 1,
  .base = (uint64_t)&idt
};


void load_idt(void) {
  ASMV("lidt %0" :: "m" (idtr));
}
