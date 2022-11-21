#ifndef EXT2_H_
#define EXT2_H_

#include <lib/asm.h>
#include <lib/types.h>
#include <dev/dev.h>

#define EXT2_DIRECT_BLOCKS 12


#define MIN_BLOCK_SIZE 1024
#define MAX_BLOCK_SIZE 4096
#define SECTOR_SIZE 512

#define SUPERBLOCK_LBA 2
#define ROOT_INODE_NUMBER 2

#define EXT2_MAGIC 0xEF53

#define EXT2_S_IFSOCK   0xC000
#define EXT2_S_IFLNK    0xA000
#define EXT2_S_IFREG    0x8000
#define EXT2_S_IFBLK    0x6000
#define EXT2_S_IFDIR    0x4000
#define EXT2_S_IFCHR    0x2000
#define EXT2_S_IFIFO    0x1000

typedef struct {
  uint32_t total_inodes;
  uint32_t total_blocks;
  uint32_t su_blocks;
  uint32_t free_blocks;
  uint32_t free_inodes;
  uint32_t superblock_idx;
  uint32_t log2block_size;
  uint32_t log2frag_size;
  uint32_t blocks_per_group;
  uint32_t frags_per_group;
  uint32_t inodes_per_group;

  uint32_t mtime;
  uint32_t wtime;

  uint16_t mount_count;
  uint16_t mount_allowed_count;
  uint16_t ext2_magic;
  uint16_t fs_state;
  uint16_t err;
  uint16_t minor;

  uint32_t last_check;
  uint32_t interval;
  uint32_t os_id;
  uint32_t major;

  uint16_t r_userid;
  uint16_t r_groupid;

  // Extended features (unsued for now).
  uint32_t first_inode;
  uint16_t inode_size;
  uint16_t superblock_group;
  uint32_t optional_feature;
  uint32_t required_feature;
  uint32_t readonly_feature;
  char fs_id[16];
  char vol_name[16];
  char last_mount_path[64];
  uint32_t compression_method;
  uint8_t file_pre_alloc_blocks;
  uint8_t dir_pre_alloc_blocks;
  uint16_t unused1;
  char journal_id[16];
  uint32_t journal_inode;
  uint32_t journal_device;
  uint32_t orphan_head;
} _packed superblock_t;

typedef struct {
  uint32_t block_bitmap;
  uint32_t inode_bitmap;
  uint32_t inode_table;
  uint32_t free_blocks;
  uint32_t free_inodes;
  uint32_t num_dirs;
  uint32_t unused1;
  uint32_t unused2;
} _packed bgd_t;

typedef struct {
  uint32_t inode;
  uint16_t size;
  uint8_t  name_len;
  uint8_t  type;
  char name[255];
} _packed direntry_t;


typedef struct {
  uint16_t permission;
  uint16_t userid;
  uint32_t size;
  uint32_t atime;
  uint32_t ctime;
  uint32_t mtime;
  uint32_t dtime;
  uint16_t gid;
  uint16_t hard_links;
  uint32_t num_sectors;
  uint32_t flags;
  uint32_t os_specific1;
  uint32_t blocks[EXT2_DIRECT_BLOCKS + 3];
  uint32_t generation;
  uint32_t file_acl;
  union {
    uint32_t dir_acl;
    uint32_t size_high;
  };
  uint32_t f_block_addr;
  char os_specific2[12];
} _packed inode_t;

typedef struct {
  uint32_t block;
  uint32_t times;
  uint8_t dirty;
  char* block_data;
} ext2_cache_t;


typedef struct ext2_fs {
  dev_driver_t* block_driver;      // Driver descriptor for device.
  superblock_t* sb;
  bgd_t* bgds;
  uint32_t block_size;
  uint32_t inode_size;
  uint32_t sectors_per_block;
  uint32_t blocks_per_group;
  uint32_t inodes_per_group;
  uint32_t total_groups;
  uint32_t bgd_blocks;
} ext2_fs_t;

void ext2_init(void);

#endif

