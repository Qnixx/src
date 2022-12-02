#include <lib/log.h>
#include <intr/init.h>

void _start(void) {
  printk("Beginning boot process..\n");
  init_interrupts();
  while (1);
}
