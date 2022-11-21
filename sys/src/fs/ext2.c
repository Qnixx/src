#include <fs/ext2.h>
#include <fs/vfs.h>
#include <lib/log.h>
#include <lib/assert.h>
#include <lib/module.h>
#include <lib/panic.h>
#include <lib/string.h>
#include <lib/math.h>
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

static uint8_t* read_block(uint64_t block) {
  uint16_t* buf = (uint16_t*)vmm_alloc_page();
  disk_read_lba(fs.block_driver, block * fs.sectors_per_block, fs.sectors_per_block, buf);
  return (uint8_t*)buf;
}


/*
static void __write_block(uint64_t block, uint16_t* buf) {
  disk_write_lba(fs.block_driver, block * fs.sectors_per_block, fs.sectors_per_block, buf);
}
*/


static inode_t* read_inode(unsigned int inode_idx, uint8_t** buf) {
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


/*
static void __write_inode(unsigned int inode_idx, inode_t* inode) {
  int group = inode_idx / fs.inodes_per_group;
  int inode_table_block = fs.bgds[group].inode_table;
  int idx_in_group = inode_idx - group * fs.inodes_per_group;
  int block_offset = (idx_in_group - 1) * fs.inode_size / fs.block_size;
  __write_block(inode_table_block + block_offset, (uint16_t*)inode);
}


static void __rewrite_bgds(void) {
  for (uint32_t i = 0; i < fs.bgd_blocks; ++i) {
    __write_block(2, (void*)fs.bgds + i * fs.block_size);
  }
}
*/


/*
 *  Allocate an inode and 
 *  return the inode index.
 *
 */

/*
static uint32_t allocate_inode(void) {
  for (size_t i = 0; i < fs.total_groups; ++i) {
    if (fs.bgds[i].free_inodes) {
      // Read the block that contains the inode bitmap.
      uint32_t inode_bitmap_block = fs.bgds[i].inode_bitmap;
      uint32_t* buf = (uint32_t*)read_block(inode_bitmap_block);
      
      for (uint32_t j = 0; j < fs.block_size / 4; ++j) {
        uint32_t bitmap = buf[j];

        if (bitmap == ~(0)) {
          continue;
        }

        for (uint32_t k = 0; k < 32; ++k) {
          uint8_t free = !((bitmap >> k) & 1);

          if (free) {
            buf[i] |= (1 << k);
            __write_block(inode_bitmap_block, (void*)buf);
            
            // Decrement free inode count.
            --fs.bgds[i].free_inodes;
            __rewrite_bgds();
            return i * fs.inodes_per_group + j * 32 + k;
          }
        }
      }
    }
  }
  
  // No more inodes!
  return (uint32_t)~(0);
}


static uint32_t alloc_block(void) {
  for (uint32_t i = 0; i < fs.total_groups; ++i) {
    if (fs.bgds[i].free_blocks) {
      uint32_t bitmap_block = fs.bgds[i].block_bitmap;
      uint32_t* buf = (uint32_t*)read_block(bitmap_block);
      for (uint32_t j = 0; j < fs.block_size/4; ++j) {
        uint32_t bitmap = buf[j];

        if (bitmap == ~(0)) {
          continue;
        }

        for (uint32_t k = 0; k < 32; ++j) {
          buf[j] |= (1 << k);
          __write_block(bitmap_block, (void*)buf);

          --fs.bgds[i].free_blocks;
          __rewrite_bgds();
          return i * fs.blocks_per_group + j * 32 + k;
        }
      }
    }
  }

  return (uint32_t)~(0);
}
*/

// Subject to removal or change.
static void __read_dirnames(unsigned int inode_idx, uint8_t** buf) {
  inode_t* inode = read_inode(inode_idx, buf);
  direntry_t* orig_direntry = (direntry_t*)read_block(inode->blocks[0]);
  direntry_t* direntry = orig_direntry;

  while (direntry->name[0]) {
    printk("[%s]: Found file => %s\n", MODULE_NAME, direntry->name);
    direntry = (direntry_t*)((uint8_t*)direntry + direntry->rec_length);
  }

  free_buf(orig_direntry);
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
  
  // rename_root("lost+found", "bruh");
  
  __read_dirnames(ROOT_INODE_NUMBER, &buf);

  free_buf(buf);
}
