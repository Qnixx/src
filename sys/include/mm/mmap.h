#ifndef MMAP_H_
#define MMAP_H_

#include <sys/mman.h>     /* for prot flags */
#include <lib/types.h>


void* k_mmap(void* addr, size_t n_pages, uint8_t prot);
void k_munmap(void* addr, size_t n_pages);

#endif
