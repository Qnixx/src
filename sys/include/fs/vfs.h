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
  size_t size;        /* Size of the file system in blocks */
  size_t blocksize;   /* Blocksize for this filesystem */
} vfs_superblock_t;

typedef struct {
  create_t create_file;
} fs_ops_t;

typedef struct VFS_FS {
  vfs_superblock_t superblock;
  fs_ops_t ops;
} fs_t;


void vfs_init(void);
void vfs_mount(fs_t* fs, const char* mountpoint);

/*
 *  n_filenames should be set to the address
 *  of a size_t variable which would then hold
 *  the amount of filenames in the path.
 *
 *  is_dir should be set to the address
 *  of a uint8_t which will then hold
 *  a 1 if the path points to a directory or a
 *  0 if it doesn't.
 *
 */

char** vfs_parse_path(const char* path, size_t* n_filenames, uint8_t* is_dir);

#endif
