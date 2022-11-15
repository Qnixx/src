#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/string.h>
#include <lib/log.h>
#include <lib/module.h>
#include <lib/assert.h>


MODULE("vfs");


static vfs_node_t* root = NULL;

static uint8_t mountpoint_exists(const char* name) {
  for (size_t i = 0; i < root->n_children; ++i) {
    if (kstrcmp(root->children[i]->filename, name) == 0) {
      return 1;
    }
  }

  return 0;
}


uint8_t file_exists(struct VFSNode* n, const char* name) {
  for (size_t i = 0; i < n->n_children; ++i) {
    if (kstrcmp(n->children[i]->filename, name) == 0) {
      return 1;
    }
  }

  return 0;
}


void vfs_init_node(vfs_node_t* node) {
  kmemcpy(&node->filename, "/", 1);
  
  node->children = kmalloc(sizeof(vfs_node_t));
  node->n_children = 0;
}


void vfs_push(vfs_node_t* to, vfs_node_t* n) {
  to->children[to->n_children++] = n;
  to->children = krealloc(to->children, sizeof(vfs_node_t) * (to->n_children + 2));
}


vfs_node_t* vfs_create_fs(const char* mountpoint_name) {
  ASSERT(root != NULL, "Root VFS node not setup!\n");

  if (mountpoint_exists(mountpoint_name))
    return NULL;

  vfs_node_t* new = kmalloc(sizeof(vfs_node_t));
  new->flags = VFS_FLAG_MOUNTPOINT;
  new->index = root->n_children;
  vfs_init_node(root);

  root->children[root->n_children++] = new;
  root->children = krealloc(root->children, sizeof(vfs_node_t) * (root->n_children + 2));
  return new;
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
