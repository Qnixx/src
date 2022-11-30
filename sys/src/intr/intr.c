#include <intr/intr.h>
#include <intr/irq.h>
#include <arch/x64/idt.h>
#include <proc/sched.h>

void intr_init(void) {
  register_irq_handler(0, _irq0);
  register_int_handler(0x81, __sched);
  register_int_handler(0x82, __user_process_entry);
}
