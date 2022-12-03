#include <fs/tmpfs.h>
#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/hashmap.h>
#include <lib/asm.h>
#include <lib/log.h>

#define TMPFS_BLOCKSIZE 256               // Measured in bytes.
#define TMPFS_SIZE_BLOCKS 90

typedef struct {
  uint8_t lock : 1;
  uint8_t* buf;
  vfs_node_t* node;
} tmpfs_fileheader_t;

static HASHMAP_TYPE(tmpfs_fileheader_t*) tmpfs_lookup_book = HASHMAP_INIT(256);
static size_t lookup_book_entries = 0;

static vfs_node_t* tmpfs_loc = NULL;

static int tmpfs_create_file(const char* name) {
  _unused tmpfs_fileheader_t* unused;

  if (lookup_book_entries > 0) {
    if (HASHMAP_SGET(&tmpfs_lookup_book, unused, name) != 0) return 1;
  }

  tmpfs_fileheader_t* file = kmalloc(sizeof(tmpfs_fileheader_t));
  file->buf = NULL;
  file->node = vfs_make_node(NULL, tmpfs_loc, name, 0);

  HASHMAP_SINSERT(&tmpfs_lookup_book, name, file);
  ++lookup_book_entries;
  return 0;
}

static FILE* tmpfs_open_file(const char* name) {
  tmpfs_fileheader_t* file;

  if (lookup_book_entries > 0) {
    if (HASHMAP_SGET(&tmpfs_lookup_book, file, name) == 0) return NULL;
  } else {
    return NULL;
  }
  
  file->lock = 1;
  return file;
}

static vfs_fs_t tmpfs = {
  .create = tmpfs_create_file,
  .open = tmpfs_open_file
};

void tmpfs_init(void) {
  tmpfs_loc = vfs_make_node(&tmpfs, vfs_get_root(), "tmp", 1);
  tmpfs_create_file("blah.txt");
  printk("%x\n", fopen("/tmp/blah.txty", "w"));
}
