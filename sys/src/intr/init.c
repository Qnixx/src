#include <intr/init.h>
#include <lib/log.h>
#if defined(__x86_64__)
#include <arch/x86_64/idt.h>
#include <arch/x86_64/exceptions.h>
#endif

#if defined(__x86_64__)
void init_interrupts(void) {
  load_idt();
  init_exceptions();
  printk("Finished setting up interrupts.\n");

}
#endif
