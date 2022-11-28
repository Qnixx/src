#ifndef HEAP_H_
#define HEAP_H_

#include <lib/types.h>

#define HEAP_DEBUG 1
#define HEAP_PAGES 2
#define MAX_HEAP_SIZE (PAGE_SIZE * HEAP_PAGES)

#define HEAPBLK_FREE 0
#define HEAPBLK_USED 1
#define DATA_START(mem_block) (((char*) mem_block) + sizeof(heapblk_t))

typedef struct heapblk_t {
  uint8_t type;
  size_t size;
  struct heapblk_t* next;
} heapblk_t;


void* kmalloc(size_t size);
void* krealloc(void* oldptr, size_t new_size);
void kfree(void* ptr);
void heap_init(void);


#endif
