#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/string.h>
#include <lib/log.h>
#include <lib/module.h>
#include <lib/assert.h>
#include <sys/errno.h>


MODULE("vfs");


static fs_t* mount_list = NULL;
static fs_t* mount_list_head = NULL;


static uint8_t mountpoint_exists(const char* mountpoint_name) {
  for (fs_t* current = mount_list; *current; current = current->next) {
    if (kstrcmp(current->name, mountpoint_name) == 0)
      return 1;
  }

  return 0;
}

 
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


int vfs_mountfs(fs_t* fs, const char* mountpoint) {
  size_t mountpoint_len = kstrlen(mountpoint);
  if (mountpoint_len >= VFS_FILENAME_LENGTH-1) {
    return -ENAMETOOLONG;
  }

  if (mountpoint_exists(mountpoint)) {
    return -EEXIST;
  }
  
  // Allocate a new fs node.
  mount_list_head->next = kmalloc(sizeof(fs_t));
  mount_list_head = mount_list_head->next;
  mount_list_head->flags |= VFS_FLAG_MOUNTPOINT;
  memcpy(mount_list_head->name, mountpoint, mountpoint_len);
  return 0;
}


void vfs_init(void) {
  // Allocate memory for the mountlist.
  mount_list = kmalloc(sizeof(fs_t));
  mount_list->next = NULL;
  kmemcpy(mount_list->name, "/", 2);
  mount_list_head = mount_list;

  PRINTK_SERIAL("[%s]: Finished setting up VFS root mountpoint.\n", MODULE_NAME);
}
