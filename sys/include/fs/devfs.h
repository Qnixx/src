#ifndef DEVFS_H_
#define DEVFS_H_

#include <lib/types.h>
#include <fs/vfs.h>

#define DEVFS_OPEN_RETVAL NULL

void devfs_init(void);
int register_chrdev(const char* name, file_ops_t* fops);

#endif
