#include <lib/log.h>
#include <intr/init.h>
#include <mm/pmm.h>
#include <fs/vfs.h>
#include <fs/tmpfs.h>
#include <fs/devfs.h>
#include <drivers/serial.h>
#include <acpi/acpi.h>
#include <arch/x86/apic/ioapic.h>
#include <arch/x86/apic/lapic.h>


static void init_mm(void) {
  pmm_init();
}

static void init_fs(void) {
  vfs_init();
  tmpfs_init();
  devfs_init();
}


static void init_drivers(void) {
  serial_init();
}


void _start(void) {
  printk("Beginning boot process..\n");
  init_interrupts();
  init_mm();

  init_fs();
  init_drivers();

  acpi_init();
  ioapic_init();
  lapic_init();

  char buf[] = "Hello, World!";
  FILE* fp = fopen("/dev/serial", "w");
  fwrite(fp, buf, sizeof(buf));
  fclose(fp);

  while (1);
}
