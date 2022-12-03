#include <lib/log.h>
#include <intr/init.h>
#include <mm/pmm.h>
#include <fs/vfs.h>
#include <fs/tmpfs.h>


static void init_mm(void) {
  pmm_init();
}

void _start(void) {
  printk("Beginning boot process..\n");
  init_interrupts();
  init_mm();
  vfs_init();
  tmpfs_init();
  while (1);
}
