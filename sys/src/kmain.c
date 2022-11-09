#include <drivers/video/framebuffer.h>
#include <lib/log.h>

__attribute__((noreturn)) void _start(void) { 
  framebuffer_init();

  while (1);
}
