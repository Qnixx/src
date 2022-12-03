#include <fs/vfs.h>
#include <sync/spinlock.h>
#include <mm/heap.h>
#include <lib/string.h>
#include <lib/log.h>
#include <lib/asm.h>

_unused static spinlock_t lock = 0;
static vfs_node_t* vfs_root = NULL;
static HASHMAP_TYPE(vfs_fs_t*) filesystems;


_unused static uint8_t is_path_valid(const char* path) {
  for (const char* ptr = path; *ptr; ++ptr) {
    switch (*ptr) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '/':
      case '-':
      case '_':
        break;
      default:
        if (*ptr >= 'A' && *ptr <= 'Z') break;
        if (*ptr >= 'a' && *ptr <= 'z') break;
        return 0;
    }
  }

  return 1;
}


static inline vfs_node_t* get_node(vfs_node_t* parent, const char* name) {
  vfs_node_t* node;

  if (parent->n_children == 0) {
    return NULL;
  }

  if (!(HASHMAP_GET(&parent->children, node, name, kstrlen(name)))) return NULL;
  return node;
}


// A fs mountpoint could be '/tmp' for example.
// '/' does not count!!
static vfs_node_t* get_fs_mountpoint(const char* path) {
  if (*path == '/') ++path;

  size_t bufidx = 0;
  char* buf = kmalloc(kstrlen(path) + 1);
  vfs_node_t* ret = NULL;

  const char* ptr = path;

  while (1) {
    if (*ptr == '/' || *ptr == '\0') {
      buf[bufidx++] = '\0';
      ret = get_node(vfs_root, buf);
      kfree(buf);
      return ret;
    }

    buf[bufidx++] = *ptr;
    ++ptr;
  }
}

vfs_node_t* vfs_get_root(void) {
  return vfs_root;
}

vfs_node_t* vfs_path_to_node(const char* path) {
  if (*path == '/') ++path;

  vfs_node_t* last_parent = vfs_root;
  size_t bufidx = 0;
  char* buf = kmalloc(kstrlen(path) + 1);
  buf[kstrlen(path)] = 0;
  
  const char* ptr = path;
  
  while (1) {
    if (*ptr == '\0' || (*ptr == '/' && *(ptr + 1) != '\0')) {
      buf[bufidx++] = 0;
      last_parent = get_node(last_parent, buf);
      if (last_parent == NULL) {
        kfree(buf);
        return NULL;
      }

      if (*ptr == '\0') {
        kfree(buf);
        break;
      }

      bufidx = 0;
      ++ptr;
    }

    buf[bufidx++] = *ptr;
    ++ptr;
  }
  

  return last_parent != vfs_root ? last_parent : NULL;
}


FILE* fopen(const char* path) { 
  vfs_node_t* mountpoint = get_fs_mountpoint(path);
  if (mountpoint == NULL) return NULL;

  vfs_node_t* node = vfs_path_to_node(path);
  if (node == NULL) return NULL;
  return mountpoint->filesystem->open(node->name);
}


vfs_node_t* vfs_make_node(vfs_fs_t* fs, vfs_node_t* parent, const char* name, uint8_t is_dir) {
  vfs_node_t* node = kmalloc(sizeof(vfs_node_t));
  node->name = kmalloc(kstrlen(name) + 1);
  kmemcpy(node->name, name, kstrlen(name));

  node->parent = parent;
  node->filesystem = fs;

  if (is_dir) {
    node->children = (typeof(node->children))HASHMAP_INIT(256);
    node->n_children = 0;
  } 

  if (parent != NULL) {
    HASHMAP_INSERT(&parent->children, name, kstrlen(name), node);
    ++parent->n_children;
  }

  return node;
}

void vfs_init(void) {
  vfs_root = vfs_make_node(NULL, NULL, "", 1);
  filesystems = (typeof(filesystems))HASHMAP_INIT(256);
  printk("[vfs]: Finished setting up.\n");
}
