#include <drivers/video/framebuffer.h>
#include <drivers/serial.h>
#include <drivers/net/rtl8139.h>
#include <drivers/timer/pit.h>
#include <drivers/hdd/ahci.h>
#include <lib/log.h>
#include <lib/string.h>
#include <lib/module.h>
#include <lib/logo.h>
#include <mm/pmm.h>
#include <arch/x86/gdt.h>
#include <arch/x64/idt.h>
#include <arch/x86/apic/lapic.h>
#include <arch/x86/apic/ioapic.h>
#include <intr/intr.h>
#include <mm/heap.h>
#include <firmware/acpi/acpi.h>
#include <net/udp.h>
#include <net/if.h>
#include <proc/proc.h>

MODULE("kmain");

static void init_mm(void) {
  pmm_init();
  heap_init();
  printk("[%s]: Memory managers initialized.\n", MODULE_NAME);
}


// TODO: Move this somewhere else.
static void init_drivers(void) {
  rtl8139_init();
  init_pit();
  ahci_init();
}


_noreturn void _start(void) { 
  framebuffer_init();
  serial_init();
  printk(" -- Welcome to Qnixx --\n");
  printk(logo);

  init_mm();
  load_gdt();
  load_idt();

  init_interrupts();
  printk("[%s]: Interrupts initialized.\n", MODULE_NAME);
  acpi_init();
  lapic_init();

  ioapic_init();

  init_drivers();
  // ASMV("sti");

  // if_manager_init();
  
  // uint8_t payload[4] = { 0x00, 0x05, 0x00, 0x01 };
  // icmp_send_msg(IPv4(192, 168, 1, 152), 8, 0, payload, sizeof(payload));
  // 192.168.1.166
  // udp_send(IPv4(192, 168, 1, 152), payload, sizeof(payload));
  tasking_init();

  while (1);
}
