#include <lib/module.h>
#include <lib/asm.h>
#include <lib/types.h>
#include <tty/console.h>
#include <drivers/video/framebuffer.h>
#include <arch/x64/idt.h>
#include <arch/x86/exceptions.h>
#include <arch/x86/gdt.h>
#include <arch/x86/tss.h>
#include <mm/pmm.h>
#include <mm/heap.h>
#include <proc/proc.h>
#include <firmware/acpi/acpi.h>
#include <arch/x86/apic/ioapic.h>
#include <arch/x86/apic/lapic.h>

MODULE_NAME("kmain");
MODULE_DESCRIPTION("Kernel startup module");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");


static void init_mm(void) {
  pmm_init();
  heap_init();
  vnprintk("Memory managers initialized.\n", 0);
}


__attribute__((noreturn)) void _start(void) {
  framebuffer_init();
  vprintk(" -- Welcome to Qnixx --\n\n");

  load_idt();
  init_exceptions();
  load_gdt();

  // Cool ASCII art logo :~)
  vprintk("   ____\n  6MMMMb            68b\n 8P    Y8           Y89\n6M      Mb ___  __  ___ ____   ___ ____   ___\nMM      MM `MM 6MMb `MM `MM(   )P' `MM(   )P'\nMM      MM  MMM9 `Mb MM  `MM` ,P    `MM` ,P\nMM      MM  MM'   MM MM   `MM,P      `MM,P\nMM      MM  MM    MM MM    `MM.       `MM.\nYM      M9  MM    MM MM    d`MM.      d`MM.\n 8b    d8   MM    MM MM   d' `MM.    d' `MM.\n  YMMMM9   _MM_  _MM_MM__d_  _)MM_ _d_  _)MM_\n    MM\n    YM.\n     `Mo\n\n");

  init_mm();
  tss_init();

  acpi_init();

  __ioapic_init();
  __lapic_init();

  tasking_init();

  while (1);
}
