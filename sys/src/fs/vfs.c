#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/string.h>
#include <lib/log.h>
#include <lib/module.h>
#include <lib/assert.h>


MODULE("vfs");


static vfs_node_t* root = NULL;

 
char** vfs_parse_path(const char* path, size_t* n_filenames, uint8_t* is_dir) {
  // Current path (path0/path1/path3)
  char current_filename[VFS_FILENAME_LENGTH];
  size_t current_filename_idx = 0;
  
  // List of paths.
  char** path_list = kmalloc(sizeof(char*));
  size_t n_path_seperators = 0;

  const char* ptr;
  for (ptr = path; *ptr; ++ptr) {
    if (current_filename_idx >= VFS_FILENAME_LENGTH-1) {
      // Path too long!
      kfree(path_list);
      return NULL;
    }

    if (*ptr == '/') {
      current_filename[current_filename_idx] = '\0';      // Null terminate.
      kmemcpy(&path_list[n_path_seperators++], current_filename, kstrlen(current_filename));
      path_list = krealloc(path_list, sizeof(char*) * (n_path_seperators + 2));
      
      // Reset the current filename.
      kmemzero(current_filename, sizeof(current_filename));
      current_filename_idx = 0;
      continue;
    }

    current_filename[current_filename_idx++] = *ptr;
  }

  if (*(ptr - 1) == '/' && is_dir != NULL) {
    *is_dir = 1;
  } else if (is_dir != NULL) {
    *is_dir = 0;
  }

  if (n_filenames != NULL) {
    *n_filenames = n_path_seperators-1;
  }

  return path_list;
}


void vfs_init_node(vfs_node_t* node) {
  node->children = kmalloc(sizeof(vfs_node_t));
  node->n_children = 0;
}


void vfs_init(void) {
  // Allocate memory for the root mountpoint and set it
  // up.
  root = kmalloc(sizeof(vfs_node_t));
  root->flags = VFS_FLAG_MOUNTPOINT;
  root->index = 0;
  vfs_init_node(root);

  PRINTK_SERIAL("[%s]: Finished setting up VFS root mountpoint.\n", MODULE_NAME);
}
