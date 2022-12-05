#include <fs/tmpfs.h>
#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/hashmap.h>
#include <lib/asm.h>
#include <lib/log.h>
#include <lib/string.h>

#define TMPFS_BLOCKSIZE 256               // Measured in bytes.
#define TMPFS_SIZE_BLOCKS 90

typedef struct {
  uint8_t lock : 1;
  uint8_t* buf;
  size_t size;
  vfs_node_t* node;
} tmpfs_fileheader_t;

static HASHMAP_TYPE(tmpfs_fileheader_t*) tmpfs_lookup_book = HASHMAP_INIT(256);
static size_t lookup_book_entries = 0;
static vfs_node_t* tmpfs_loc = NULL;
static vfs_fs_t tmpfs;


static void* tmpfs_open_file(const char* name);
static void tmpfs_read(void* fs_node, char* buf, size_t n_bytes);
static void tmpfs_write(void* fs_node, char* buf, size_t n_bytes);
static void tmpfs_close_file(void* fs_node);

static file_ops_t default_file_ops = {
  .open = tmpfs_open_file,
  .read = tmpfs_read,
  .write = tmpfs_write,
  .close = tmpfs_close_file
};

static int tmpfs_create_file(const char* name) {
  _unused tmpfs_fileheader_t* unused;

  if (lookup_book_entries > 0) {
    if (HASHMAP_SGET(&tmpfs_lookup_book, unused, name) != 0) return 1;
  }

  tmpfs_fileheader_t* file = kmalloc(sizeof(tmpfs_fileheader_t));
  file->buf = kmalloc(2);
  file->node = vfs_make_node(&tmpfs, tmpfs_loc, name, 0, &default_file_ops);

  HASHMAP_SINSERT(&tmpfs_lookup_book, name, file);
  ++lookup_book_entries;
  return 0;
}

static void* tmpfs_open_file(const char* name) {
  tmpfs_fileheader_t* file;

  if (lookup_book_entries > 0) {
    if (HASHMAP_SGET(&tmpfs_lookup_book, file, name) == 0) return NULL;
  } else {
    return NULL;
  }

  /* Don't open the file if already opened */
  /* TODO: Add lock only per process */
  if (file->lock) return NULL;
  
  file->lock = 1;
  return file;
}

static void tmpfs_close_file(void* fs_node) {
  tmpfs_fileheader_t* file = (tmpfs_fileheader_t*)fs_node;
  file->lock = 0;
}

static void tmpfs_read(void* fs_node, char* buf, size_t n_bytes) {
  tmpfs_fileheader_t* file = (tmpfs_fileheader_t*)fs_node; 

  /* Don't read if file lock not acquired */
  if (!(file->lock)) return;

  for (size_t i = 0; i < n_bytes; ++i) { 
    buf[i] = file->buf[i];
  }
}


static void tmpfs_write(void* fs_node, char* buf, size_t n_bytes) {
  tmpfs_fileheader_t* file = (tmpfs_fileheader_t*)fs_node; 

  /* Don't write if file lock not acquired */
  if (!(file->lock)) return;

  for (size_t i = 0; i < n_bytes; ++i) {
    if (i >= file->size-1) {
      file->size += 2;
      file->buf = krealloc(file->buf, file->size);
    }

    file->buf[i] = buf[i];
  }
}

void tmpfs_init(void) {
  tmpfs.fops = kmalloc(sizeof(file_ops_t));
  
  kmemzero(tmpfs.fops, sizeof(file_ops_t));      /* Make sure every field is NULL */
  tmpfs.fops->create = tmpfs_create_file;
  tmpfs.fops->open = tmpfs_open_file;
  tmpfs.default_fops = &default_file_ops;

  tmpfs_loc = vfs_make_node(&tmpfs, vfs_get_root(), "tmp", 1, tmpfs.fops);
}
