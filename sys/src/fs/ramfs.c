#include <fs/ramfs.h>
#include <fs/vfs.h>
#include <lib/log.h>
#include <lib/module.h>

MODULE("ramfs");

// 1K block size.
#define BLOCKSIZE 1024


static int create_file(fs_t* fs, const char* path) {
  return 0;
}


static fs_descriptor_t ramfs = {
  .size = 0,
  .blocksize = BLOCKSIZE,
  .ops = { create_file }
};


void ramfs_init(const char* mountpoint) {
  vfs_mountfs(mountpoint, &ramfs);
  printk("[%s]: Mounted %s\n", MODULE_NAME, mountpoint);
}
