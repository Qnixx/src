#include <fs/devfs.h>
#include <mm/heap.h>
#include <lib/asm.h>
#include <lib/hashmap.h>
#include <lib/string.h>
#include <lib/log.h>

static vfs_fs_t devfs;
static vfs_node_t* devfs_loc = NULL;

typedef struct {
  vfs_node_t* node;
  file_ops_t* fops;
  uint8_t lock;
} devfs_fileheader_t;

static HASHMAP_TYPE(devfs_fileheader_t*) devfs_lookup_book = HASHMAP_INIT(355);
static size_t lookup_book_entries = 0;


/* 
 * This will not do anything since register_chrdev() needs to be
 * called instead
 *
 */
static int devfs_stub_create(const char* _unused name) { return 1; }

static void* devfs_open(const char* name) {
  devfs_fileheader_t* dev;

  if (lookup_book_entries > 0) {
    if (HASHMAP_SGET(&devfs_lookup_book, dev, name) == 0) return NULL;
  } else {
    return NULL;
  }

  // TODO: Add lock only per process.
  if (dev->lock) return NULL;
  dev->lock = 1;
  dev->fops->open(name);
  return dev;
}

static void devfs_read(void* fs_node, char* out_buf, size_t n_bytes) {
  devfs_fileheader_t* dev = (devfs_fileheader_t*)fs_node;
  if (!(dev->lock)) return;

  if (dev->fops->read) dev->fops->read(fs_node, out_buf, n_bytes);
}


static void devfs_write(void* fs_node, char* in_buf, size_t n_bytes) {
  devfs_fileheader_t* dev = (devfs_fileheader_t*)fs_node;
  if (!(dev->lock)) return;
  if (dev->fops->write) dev->fops->write(fs_node, in_buf, n_bytes);
}

static file_ops_t default_file_ops = {
  .open = devfs_open,
  .read = devfs_read,
  .write = devfs_write,
  .close = NULL
};

int register_chrdev(const char* name, file_ops_t* fops) {
  _unused devfs_fileheader_t* unused;

  if (lookup_book_entries > 0) {
    if (HASHMAP_SGET(&devfs_lookup_book, unused, name) != 0) return 1;
  }
  
  devfs_fileheader_t* dev = kmalloc(sizeof(devfs_fileheader_t));
  dev->fops = fops;
  dev->node = vfs_make_node(&devfs, devfs_loc, name, 0, fops);
  HASHMAP_SINSERT(&devfs_lookup_book, name, dev);
  ++lookup_book_entries;
  return 0;
}

void devfs_init(void) {
  devfs.fops = kmalloc(sizeof(file_ops_t));
  kmemzero(devfs.fops, sizeof(file_ops_t));

  devfs.fops->create = devfs_stub_create;
  devfs.fops->open = devfs_open;
  devfs.default_fops = &default_file_ops;

  devfs_loc = vfs_make_node(&devfs, vfs_get_root(), "dev", 1, devfs.fops);
}
