#include <fs/ext2.h>
#include <lib/log.h>
#include <lib/assert.h>
#include <lib/module.h>
#include <lib/panic.h>
#include <mm/vmm.h>
#include <block/disk.h>

MODULE("ext2");

static dev_driver_t* block_driver = NULL;


void ext2_init(void) {
  PRINTK_SERIAL("[%s] Locating a block driver..\n", MODULE_NAME);

  block_driver = find_driver(DEV_BLOCK, 0);
  ASSERT(block_driver != NULL, "No block driver found!\n");
  PRINTK_SERIAL("[%s]: Found block driver: %s\n", MODULE_NAME, block_driver->name);

  uint16_t* buf = vmm_alloc_page();
  disk_read_lba(block_driver, 0, 1, buf);
  printk("%x\n", buf[0]);
}
