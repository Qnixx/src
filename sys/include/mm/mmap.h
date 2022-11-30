#ifndef MMAP_H_
#define MMAP_H_

#include <lib/types.h>


#define PROT_READ (1 << 0)
#define PROT_WRITE (1 << 1)
#define PROT_USER (1 << 2)


void k_mmap(void* vaddr, size_t n_pages, uint8_t prot);


#endif // MMAP_H_
