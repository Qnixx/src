#ifndef VFS_H_
#define VFS_H_

#include <lib/types.h>

/*
 *  Documentation for this file is in
 *  share/docs/fs/vfs.rst
 *
 */

#define VFS_FILENAME_LENGTH 150

#define VFS_FLAG_MOUNTPOINT (1 << 0)
#define VFS_FLAG_FILE (1 << 1)
#define VFS_FLAG_DIRECTORY (1 << 2)

struct VFS_FS;

typedef int(*create_t)(struct VFS_FS* fs, const char* path);

typedef struct {
  create_t create_file;
} fs_ops_t;

typedef struct {
  size_t size;        /* Size of the file system in blocks */
  size_t blocksize;   /* Blocksize for this filesystem */
  fs_ops_t ops;
} fs_descriptor_t;

typedef struct VFS_FS {
  char* path;
  uint16_t flags;
  fs_descriptor_t* desc;
  struct VFS_FS* next;
} fs_t;


typedef struct ParsedPath {
  char filename[VFS_FILENAME_LENGTH];
  uint8_t is_dir;
  struct ParsedPath* next;
} parsed_path_t;


void vfs_init(void);
int vfs_mountfs(const char* mountpoint, fs_descriptor_t* desc);


parsed_path_t* vfs_parse_path(const char* path);
void vfs_free_parse_nodes(parsed_path_t* base);

#endif
