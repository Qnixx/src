#include <lib/limine.h>
#include <lib/module.h>
#include <lib/asm.h>
#include <lib/types.h>
#include <tty/console.h>
#include <drivers/video/framebuffer.h>
#include <arch/x64/idt.h>

MODULE_NAME("kmain");
MODULE_DESCRIPTION("Kernel startup module");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");


__attribute__((noreturn)) void _start(void) {
  load_idt();
  framebuffer_init();

  for (size_t i = 0; i < 50; ++i) {
    vprintk("Hello!\n", i);
    CLI_SLEEP;
  }

  for (size_t i = 0; i < 50; ++i) {
    vprintk("Cats go meow!\n", i);
    CLI_SLEEP;
  }

  while (1);
}
