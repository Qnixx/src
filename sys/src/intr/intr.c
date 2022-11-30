#include <intr/intr.h>
#include <intr/irq.h>
#include <arch/x64/idt.h>

void intr_init(void) {
  register_irq_handler(0, _irq0);
}
