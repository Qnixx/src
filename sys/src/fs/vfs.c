#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/string.h>
#include <lib/log.h>
#include <lib/module.h>
#include <lib/assert.h>
#include <sys/errno.h>

#define VFS_DEBUG 1


MODULE("vfs");


static fs_t* mount_list = NULL;
static fs_t* mount_list_head = NULL;


static uint8_t mountpoint_exists(const char* mountpoint) {
  for (fs_t* current = mount_list; current; current = current->next) {
    if (kstrcmp(current->path, mountpoint) == 0)
      return 1;
  }

  return 0;
}


void vfs_free_parse_nodes(parsed_path_t* base) {
  while (base) {
    parsed_path_t* tmp = base->next;
    kfree(base);
    base = tmp;
  }
}


static uint8_t is_valid_path(const char* path) {
  for (const char* ptr = path; *ptr; ++ptr) {
    char lowercase = (*ptr | (1 << 5));
    if (lowercase < 'a' || lowercase > 'z') {
      return 0;
    }
  }

  return 1;
}

 
parsed_path_t* vfs_parse_path(const char* path) {
  if (!(is_valid_path(path))) {
    return NULL;
  }

  char current_filename[VFS_FILENAME_LENGTH];
  size_t path_len = kstrlen(path);

  // Parse nodes.
  parsed_path_t* base_node = kmalloc(sizeof(parsed_path_t));
  parsed_path_t* current_node = base_node;
  base_node->next = NULL;


  size_t cfname_idx = 0;      // Current filename index.
  for (size_t i = 0; i < path_len; ++i) {
    if (cfname_idx >= VFS_FILENAME_LENGTH) {
      vfs_free_parse_nodes(base_node);
    }

    if (path[i] == '/') {
      current_filename[cfname_idx] = '\0';
      kmemcpy(current_node->filename, current_filename, kstrlen(current_filename));
      kmemzero(current_filename, sizeof(current_filename));
      cfname_idx = 0;

      current_node->is_dir = 1;
      
      // Make a new node.
      current_node->next = kmalloc(sizeof(parsed_path_t));
      current_node = current_node->next;
      current_node->next = NULL;
      continue;
    } else if (i == path_len-1) {
      current_node->next = NULL;
      current_node->is_dir = 0;
      kmemcpy(current_node->filename, current_filename, kstrlen(current_filename));
      break;
    }

    current_filename[cfname_idx++] = path[i];
  }

  return base_node;
}


int vfs_mountfs(const char* mountpoint, fs_descriptor_t* desc) {
  if (mountpoint_exists(mountpoint) || !(is_valid_path(mountpoint))) {
    return 1;
  }

  mount_list_head->next = kmalloc(sizeof(fs_t));
  mount_list_head = mount_list_head->next;
  mount_list_head->next = NULL;

  size_t mountpoint_len = kstrlen(mountpoint);
  mount_list_head->path = kmalloc(sizeof(char*) * mountpoint_len);
  kmemcpy(mount_list_head->path, mountpoint, mountpoint_len);
  return 0;
}


void vfs_init(void) {
  // Allocate memory for the mountlist.
  mount_list = kmalloc(sizeof(fs_t));
  mount_list->next = NULL;
  mount_list->path = "/";
  mount_list_head = mount_list;

  PRINTK_SERIAL("[%s]: Mounted '/'\n", MODULE_NAME);
}
