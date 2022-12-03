#ifndef FS_VFS_H_
#define FS_VFS_H_


#include <lib/types.h>
#include <lib/hashmap.h>

struct vfs_fs;

typedef struct vfs_node {
  struct vfs_fs* filesystem;
  char* name;
  struct vfs_node* parent;
  size_t n_children;
  HASHMAP_TYPE(struct vfs_node*) children;
} vfs_node_t;

typedef void FILE;


typedef struct vfs_fs {
  int(*create)(const char* name);
  FILE*(*open)(const char* name);
} vfs_fs_t;


vfs_node_t* vfs_make_node(vfs_fs_t* fs, vfs_node_t* parent, const char* name, uint8_t is_dir);
vfs_node_t* vfs_get_root(void);
void vfs_init(void);

FILE* fopen(const char* path, const char* mode);


#endif
