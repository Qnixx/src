#include <fs/ext2.h>
#include <fs/vfs.h>
#include <lib/log.h>
#include <lib/assert.h>
#include <lib/module.h>
#include <lib/panic.h>
#include <mm/vmm.h>
#include <block/disk.h>


static int create(fs_t* fs, const char* path) {
  return 0;
}


// TODO: Get rid of unused attribute.
__attribute__((unused)) static fs_descriptor_t desc = {
  .size = 0,
  .blocksize = 0,
  .ops = { create }
};

MODULE("ext2");

static dev_driver_t* block_driver = NULL;


void ext2_init(void) {
  printk("[%s] Locating a block driver..\n", MODULE_NAME);

  block_driver = find_driver(DEV_BLOCK, 0);
  ASSERT(block_driver != NULL, "No block driver found!\n");
  printk("[%s]: Found block driver: %s\n", MODULE_NAME, block_driver->name);


}
