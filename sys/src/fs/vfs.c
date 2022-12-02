#include <fs/vfs.h>
#include <sync/spinlock.h>
#include <mm/heap.h>
#include <lib/string.h>
#include <lib/log.h>

static spinlock_t lock = 0;
static vfs_node_t* vfs_root = NULL;
static HASHMAP_TYPE(vfs_fs_t*) filesystems;


vfs_node_t* vfs_make_node(vfs_fs_t* fs, vfs_node_t* parent, const char* name, uint8_t is_dir) {
  vfs_node_t* node = kmalloc(sizeof(vfs_node_t));
  node->name = kmalloc(kstrlen(name) + 1);
  kmemcpy(node->name, name, kstrlen(name));

  node->parent = parent;
  node->filesystem = fs;

  if (is_dir) {
    node->children = (typeof(node->children))HASHMAP_INIT(256);
  }

  return node;
}

void vfs_init(void) {
  vfs_root = vfs_make_node(NULL, NULL, "", 0);
  filesystems = (typeof(filesystems))HASHMAP_INIT(256);
  printk("[vfs]: Finished setting up.\n");
}
