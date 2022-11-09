#include <drivers/video/framebuffer.h>
#include <drivers/serial.h>
#include <lib/log.h>

__attribute__((noreturn)) void _start(void) { 
  framebuffer_init();
  serial_init();
  printk(" -- Welcome to Qnixx --\n\n");

  while (1);
}
