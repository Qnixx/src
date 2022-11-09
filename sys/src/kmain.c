#include <drivers/video/framebuffer.h>
#include <drivers/serial.h>
#include <lib/log.h>
#include <mm/pmm.h>

static void init_mm(void) {
  pmm_init();
}


__attribute__((noreturn)) void _start(void) { 
  framebuffer_init();
  serial_init();
  printk(" -- Welcome to Qnixx --\n\n");

  init_mm();

  while (1);
}
