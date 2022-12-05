#ifndef FS_VFS_H_
#define FS_VFS_H_


#include <lib/types.h>
#include <lib/hashmap.h>

struct vfs_fs;


typedef struct {
  int(*create)(const char* name);
  void*(*open)(const char* name);
  void(*read)(void* fs_node, char* buf, size_t n_bytes);
  void(*write)(void* fs_node, char* buf, size_t n_bytes);
  void(*close)(void* fs_node);
} file_ops_t;

typedef struct vfs_node {
  struct vfs_fs* filesystem;
  char* name;
  struct vfs_node* parent;
  size_t n_children;
  file_ops_t* fops;
  HASHMAP_TYPE(struct vfs_node*) children;
} vfs_node_t;

typedef struct {
  vfs_node_t* vfs_node;
  void* fs_node;       /* Could point to any structure the filesystem defines */
} FILE;

typedef struct vfs_fs {
  file_ops_t* fops;
  file_ops_t* default_fops;   /* File operations for each file in this filesystem */
} vfs_fs_t;

vfs_node_t* vfs_make_node(vfs_fs_t* fs, vfs_node_t* parent, const char* name, uint8_t is_dir, file_ops_t* fops);
vfs_node_t* vfs_get_root(void);
void vfs_init(void);

FILE* fopen(const char* path, const char* mode);
void fread(FILE* stream, char* out_ptr, size_t n_bytes);
void fwrite(FILE* stream, char* in_ptr, size_t n_bytes);
void fclose(FILE* stream);

#endif
