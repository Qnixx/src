#include <drivers/video/framebuffer.h>
#include <drivers/serial.h>
#include <lib/log.h>
#include <lib/module.h>
#include <mm/pmm.h>
#include <arch/x64/idt.h>
#include <intr/intr.h>

MODULE("kmain");

static void init_mm(void) {
  pmm_init();
  printk("[%s]: Memory managers initialized.\n", MODULE_NAME);
}


__attribute__((noreturn)) void _start(void) { 
  framebuffer_init();
  serial_init();
  printk(" -- Welcome to Qnixx --\n\n");

  init_mm();
  load_idt();

  init_interrupts();
  printk("[%s]: Interrupts initialized.\n", MODULE_NAME);

  while (1);
}
