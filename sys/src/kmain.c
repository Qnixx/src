#include <drivers/video/framebuffer.h>
#include <drivers/serial.h>
#include <lib/log.h>
#include <lib/string.h>
#include <lib/module.h>
#include <mm/pmm.h>
#include <arch/x64/idt.h>
#include <arch/x86/apic/lapic.h>
#include <arch/x86/apic/ioapic.h>
#include <intr/intr.h>
#include <drivers/net/rtl8139.h>
#include <mm/heap.h>
#include <firmware/acpi/acpi.h>
#include <net/icmp.h>

MODULE("kmain");

static void init_mm(void) {
  pmm_init();
  heap_init();
  printk("[%s]: Memory managers initialized.\n", MODULE_NAME);
}


// TODO: Move this somewhere else.
static void init_drivers(void) {
  rtl8139_init();
}


__attribute__((noreturn)) void _start(void) { 
  framebuffer_init();
  serial_init();
  printk(" -- Welcome to Qnixx --\n\n");

  init_mm();
  load_idt();

  init_interrupts();
  printk("[%s]: Interrupts initialized.\n", MODULE_NAME);
  acpi_init();
  lapic_init();

  ioapic_init();

  init_drivers();
  ASMV("sti");
  
  uint8_t payload[4] = { 0x00, 0x05, 0x00, 0x01 };
  icmp_send_msg(IPv4(192, 168, 1, 152), 8, 0, payload, sizeof(payload));

  while (1);
}
