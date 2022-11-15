#ifndef VFS_H_
#define VFS_H_

#include <lib/types.h>

#define VFS_FILENAME_LENGTH 150

#define VFS_FLAG_MOUNTPOINT (1 << 0)
#define VFS_FLAG_FILE (1 << 1)
#define VFS_FLAG_DIRECTORY (1 << 2)


struct VFSNode;

typedef int(*create_t)(struct VFSNode* _this, const char* name);

/*
 *  flags: Flags that tell the type of file this
 *         is.
 *
 *  children: Node's children.
 *  n_children: Child count.
 *  index: Index of this file into this specific node.
 *
 */

typedef struct VFSNode {
  char filename[VFS_FILENAME_LENGTH];
  uint8_t flags;
  struct VFSNode** children;
  size_t n_children;
  size_t index;
  create_t create;
} vfs_node_t;


void vfs_init(void);
void vfs_init_node(vfs_node_t* node);
void vfs_push(vfs_node_t* to, vfs_node_t* n);
uint8_t file_exists(struct VFSNode* n, const char* name);
vfs_node_t* vfs_create_fs(const char* mountpoint_name);

#endif
