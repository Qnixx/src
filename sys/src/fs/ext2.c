#include <fs/ext2.h>
#include <fs/vfs.h>
#include <lib/log.h>
#include <lib/assert.h>
#include <lib/module.h>
#include <lib/panic.h>
#include <lib/string.h>
#include <lib/math.h>
#include <lib/time.h>
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
#define BGDT_START() (fs.block_size == 1024 ? 2:1)

static uint8_t* read_block(uint64_t block) {
  uint8_t* buf = (uint8_t*)vmm_alloc_page();
  disk_read_lba(fs.block_driver, block * fs.sectors_per_block, fs.sectors_per_block, (uint16_t*)buf);
  return buf;
}

static void write_block(uint64_t block, uint8_t* buf) {
  if (block == 0) {
    printk(PRINTK_RED "[%s]: Block 0 write attempted.\n", MODULE_NAME);
    return;
  }

  disk_write_lba(fs.block_driver, block * fs.sectors_per_block, fs.sectors_per_block, (uint16_t*)buf);
}


static bgd_t* read_bgd(unsigned int bgd_idx) {
  int block = bgd_idx / fs.bgds_per_block;
  if(fs.bgdt_block != block) {
    fs.bgds = (bgd_t*)read_block(BGDT_START() + block);
    fs.bgdt_block = block;
  }

  return &fs.bgds[bgd_idx % fs.bgds_per_block];
}

static void write_bgd(unsigned int bgd_idx, bgd_t* bgd) {
  bgd_t* current = read_bgd(bgd_idx);
  kmemcpy(current, bgd, sizeof(bgd_t));

  write_block(BGDT_START() + fs.bgdt_block, (uint8_t*)fs.bgds);
}


static inode_t* read_inode(unsigned int inode_idx, uint8_t** buf) {
  if (inode_idx == 0) {
    return NULL;
  }

  int group = inode_idx / fs.sb->inodes_per_group;
  int inode_table_block = read_bgd(group)->inode_table;
  int idx_in_group = inode_idx - group * fs.sb->inodes_per_group;
  int block_offset = (idx_in_group - 1) * fs.sb->inode_size / fs.block_size;
  int offset_in_block = (idx_in_group - 1) - block_offset * (fs.block_size / fs.sb->inode_size);

  *buf = read_block(inode_table_block + block_offset);
  return (inode_t*)(*buf + offset_in_block * fs.sb->inode_size);
}

/*
 *  Allocate an inode and 
 *  return the inode index.
 *
 */
static uint32_t alloc_inode(void) {
  if(!fs.sb->free_inodes) {
    printk(PRINTK_RED "[%s] Cannot allocate inode: filesystem full\n", MODULE_NAME);
    return 0;
  }

  for (size_t i = 0; i < fs.total_groups; i++) {
    bgd_t* bgd = read_bgd(i);
    if (!bgd->free_inodes) continue;

    // Read the block that contains the inode bitmap.
    uint32_t* buf = (uint32_t*)read_block(bgd->inode_bitmap);
    for (uint32_t j = 0; j < fs.block_size / 4; j++) {
      uint32_t bitmap = buf[j];
      if (bitmap == ~(0)) continue;

      for (uint32_t k = 0; k < 32; k++) {
        uint8_t free = !((bitmap >> k) & 1);
        if (!free) continue;

        buf[j] |= (1 << k);
        write_block(bgd->inode_bitmap, (uint8_t*)buf);

        // Decrement free inode counts.
        bgd->free_inodes--;
        fs.sb->free_inodes--;
        write_bgd(i, bgd);
        disk_write_lba(fs.block_driver, SUPERBLOCK_LBA, 1, (uint16_t*)fs.sb);
        return i * fs.sb->inodes_per_group + j * 32 + k;
      }
    }
  }
  
  // No more inodes!
  return (uint32_t)~(0);
}

/*
 *  Allocate a block and 
 *  return the block index.
 *
 */
static uint32_t alloc_block(void) {
  if(!fs.sb->free_inodes) {
    printk(PRINTK_RED "[%s] Cannot allocate block: filesystem full\n", MODULE_NAME);
    return 0;
  }

  for (uint32_t i = 0; i < fs.total_groups; i++) {
    bgd_t* bgd = read_bgd(i);
    if (!bgd->free_blocks) continue;

    // Read the block that contains the block bitmap.
    uint32_t* buf = (uint32_t*)read_block(bgd->block_bitmap);
    for (uint32_t j = 0; j < fs.block_size / 4; j++) {
      uint32_t bitmap = buf[j];
      if (bitmap == ~(0)) continue;

      for (uint32_t k = 0; k < 32; k++) {
        uint8_t free = !((bitmap >> k) & 1);
        if (!free) continue;

        buf[j] |= (1 << k);
        write_block(bgd->block_bitmap, (uint8_t*)buf);

        // Decrement free block counts.
        bgd->free_blocks--;
        fs.sb->free_blocks--;
        write_bgd(i, bgd);
        disk_write_lba(fs.block_driver, SUPERBLOCK_LBA, 1, (uint16_t*)fs.sb);
        return i * fs.sb->blocks_per_group + j * 32 + k;
      }
    }
  }

  // No more blocks!
  return (uint32_t)~(0);
}


static void read_dirnames(unsigned int inode_idx) {
  uint8_t* inode_buf;
  inode_t* inode = read_inode(inode_idx, &inode_buf);
  uint8_t* data_buf = read_block(inode->blocks[0]);
  direntry_t* direntry = (direntry_t*)data_buf;

  while (direntry->name[0]) {
    uint32_t time = inode->mtime;

    // TODO: Alignment, proper uid/gid names, and permissions
    printk("%d %d %d %d %s %d %d:%d %s\n", inode->hard_links, inode->userid, inode->gid, inode->size, month_names[GET_MONTH(time)], GET_DAY(time), GET_HOUR(time), GET_MINUTE(time), direntry->name);

    direntry = (direntry_t*)((uint8_t*)direntry + direntry->rec_length);
  }

  free_buf(data_buf);
  free_buf(inode_buf);
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
  fs.bgdt_block = (uint32_t)~(0);
  fs.block_size = MIN_BLOCK_SIZE << fs.sb->log2block_size;
  fs.sectors_per_block = DIV_CEIL(fs.block_size, SECTOR_SIZE);
  fs.total_groups = DIV_CEIL(fs.sb->total_blocks, fs.sb->blocks_per_group);
  fs.bgd_blocks = DIV_CEIL(fs.total_groups * sizeof(bgd_t), fs.block_size);
  fs.bgds_per_block = fs.block_size / sizeof(bgd_t);

  desc.blocksize = fs.block_size;
  desc.size = fs.sb->total_blocks;

  // Validate filesystem
  if(fs.block_size > MAX_BLOCK_SIZE) {
    printk("[%s]: Block size %d is above %d limit.\n", MODULE_NAME, fs.block_size, MAX_BLOCK_SIZE);
    vmm_free_page(fs.sb);
    return;
  }

  // Test
  read_dirnames(ROOT_INODE_NUMBER);
}
