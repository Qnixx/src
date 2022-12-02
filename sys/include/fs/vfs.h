#ifndef FS_VFS_H_
#define FS_VFS_H_


#include <lib/types.h>
#include <lib/hashmap.h>


struct vfs_fs;

typedef struct vfs_node {
  struct vfs_node* mountpoint;
  struct vfs_node* redir;
  struct vfs_fs* filesystem;
  char* path;
  struct vfs_node* parent;
  size_t n_children;
  HASHMAP_TYPE(struct vfs_node*) children;
} vfs_node_t;


typedef struct vfs_fs {
  struct vfs_fs*(*init)(void);
} vfs_fs_t;


vfs_node_t* vfs_make_node(vfs_fs_t* fs, vfs_node_t* parent, const char* path, uint8_t is_dir);
void vfs_init(void);


#endif
