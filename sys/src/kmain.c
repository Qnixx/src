#include <lib/limine.h>
#include <lib/module.h>
#include <lib/asm.h>
#include <lib/types.h>
#include <tty/console.h>
#include <drivers/video/framebuffer.h>
#include <arch/x64/idt.h>
#include <arch/x86/exceptions.h>
#include <arch/x86/gdt.h>

MODULE_NAME("kmain");
MODULE_DESCRIPTION("Kernel startup module");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");


__attribute__((noreturn)) void _start(void) {
  load_idt();
  framebuffer_init();
  init_exceptions();
  load_gdt();

  vnprintk("Welcome to Qnixx!\n", 0);

  while (1);
}
