#ifndef HEAP_H_
#define HEAP_H_

#include <lib/types.h>


#define HEAP_SIZE_PAGES 10


void* kmalloc(size_t sz);
void* krealloc(void* oldptr, size_t sz);
void kfree(void* ptr);


#endif
