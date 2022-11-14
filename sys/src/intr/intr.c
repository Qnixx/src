#include <intr/intr.h>
#include <intr/irq.h>
#include <arch/x64/exceptions.h>
#include <arch/x64/idt.h>
#include <arch/cpu/smp.h>

#define DESTFIELD_START 56

void init_etc_irqs(void) {
  /*
   *  Set IOAPIC redirection table entry's physical destination
   *  to the Bootstrap Processor's LAPIC ID for IRQ for IRQ 0.
   *
   */
  register_irq(0, _irq0, smp_get_bsp_id() << DESTFIELD_START);
}

void init_interrupts(void) {
  init_exceptions();
}
