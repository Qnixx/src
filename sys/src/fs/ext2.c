#include <fs/ext2.h>
#include <fs/vfs.h>
#include <lib/log.h>
#include <lib/assert.h>
#include <lib/module.h>
#include <lib/panic.h>
#include <lib/string.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <block/disk.h>

MODULE("ext2");


static int create(fs_t* fs, const char* path) {
  return 0;
}

static fs_descriptor_t desc = {
  .size = 0,
  .blocksize = 0,
  .ops = { create }
};

static ext2_fs_t fs;

#define free_buf(buf) vmm_free_page((uint8_t*)buf)
// #define block_group(inode) ((inode - 1) / fs.inodes_per_group)
// #define group_index(inode) ((inode - 1) % fs.inodes_per_group)
// #define containing_block(inode) ((group_index(inode) * sizeof(inode_t)) / fs.block_size)

uint8_t* read_block(uint64_t block) {
  uint16_t* buf = (uint16_t*)vmm_alloc_page();
  disk_read_lba(fs.block_driver, block * fs.sectors_per_block, fs.sectors_per_block, buf);
  return (uint8_t*)buf;
}


static inode_t* __read_inode(unsigned int inode_idx, uint8_t** buf) {
  if (inode_idx == 0) {
    return NULL;
  }

  int group = inode_idx / fs.inodes_per_group;
  int inode_table_block = fs.bgds[group].inode_table;
  int idx_in_group = inode_idx - group * fs.inodes_per_group;
  int block_offset = (idx_in_group - 1) * fs.inode_size / fs.block_size;
  int offset_in_block = (idx_in_group - 1) - block_offset * (fs.block_size / fs.inode_size);

  *buf = read_block(inode_table_block + block_offset);
  return (inode_t*)(*buf + offset_in_block * fs.inode_size);
}



static void __read_dirnames(unsigned int inode_idx, uint8_t** buf) {
  inode_t* inode = __read_inode(inode_idx, buf);
  if (inode == NULL) {
    return;
  }
  
  char* orig_filename = kmalloc(sizeof(char*) * 256);
  char* filename = orig_filename;
  kmemzero(filename, sizeof(256));

  uint8_t* block = (uint8_t*)read_block(inode->blocks[0]);
  direntry_t* direntry = (direntry_t*)block;
  kmemcpy(filename, direntry->name, sizeof(direntry->name));
  
  while (*filename) {
    printk("Found file:  %s\n", filename);
    filename += direntry->size;
  }
  
  kfree(orig_filename);
}



void ext2_init(void) {
  printk("[%s] Locating a block driver..\n", MODULE_NAME);

  // Find a driver
  fs.block_driver = find_driver(DEV_BLOCK, 0);
  ASSERT(fs.block_driver != NULL, "No block driver found!\n");
  printk("[%s]: Found block driver: %s\n", MODULE_NAME, fs.block_driver->name);

  // Read superblock
  fs.sb = vmm_alloc_page();
  disk_read_lba(fs.block_driver, SUPERBLOCK_LBA, 1, (uint16_t*)fs.sb);

  if (fs.sb->ext2_magic != EXT2_MAGIC) {
    PRINTK_SERIAL("[%s]: This is not an ext2 filesystem!\n", MODULE_NAME);
    vmm_free_page(fs.sb);
    return;
  }
  
  // Calculate filesystem parameters
  fs.block_size = MIN_BLOCK_SIZE << fs.sb->log2block_size;
  fs.inode_size = fs.sb->inode_size;
  fs.sectors_per_block = DIV_CEIL(fs.block_size, SECTOR_SIZE);
  fs.blocks_per_group = fs.sb->blocks_per_group;
  fs.inodes_per_group = fs.sb->inodes_per_group;
  fs.total_groups = DIV_CEIL(fs.sb->total_blocks, fs.sb->blocks_per_group);
  fs.bgd_blocks = DIV_CEIL(fs.total_groups * sizeof(bgd_t), fs.block_size);

  desc.blocksize = fs.block_size;
  desc.size = fs.sb->total_blocks;

  // Validate filesystem
  if(fs.block_size > MAX_BLOCK_SIZE) {
    printk("[%s]: Block size %d is above %d limit.\n", fs.block_size, MAX_BLOCK_SIZE);
    vmm_free_page(fs.sb);
    return;
  }

  fs.bgds = (bgd_t*)read_block(fs.block_size == 1024 ? 2:1);
  uint8_t* buf = NULL;

  __read_dirnames(ROOT_INODE_NUMBER, &buf);
  free_buf(buf);
}
