#include <mm/heap.h>
#include <mm/mmap.h>
#include <lib/module.h>
#include <lib/log.h>
#include <lib/string.h>

MODULE("heap");

#define HEAP_DEBUG 0
#define HEAP_PAGES 80
#define MAX_HEAP_SIZE 0x1000*HEAP_PAGES

static heapblk_t* heap_head = NULL;
static heapblk_t* heap_tail = NULL;
static size_t bytes_allocated = 0;

static heapblk_t* first_fit(size_t size) {
  for (heapblk_t* block = heap_tail; block != NULL; block = block->next) {
    if (block->type == HEAPBLK_FREE && block->size >= size) return block;
  }

  return NULL;
}

void* kmalloc(size_t size) {
  if (size == 0) {
    printk(PRINTK_RED "[%s:%s()]: size == 0\n", MODULE_NAME, __func__);
    return NULL;
  }

  if (bytes_allocated + size >= MAX_HEAP_SIZE-1) {
    printk(PRINTK_RED "[%s:%s()]: Heap full (memory allocated: %d bytes\n", MODULE_NAME, __func__, bytes_allocated);
    return NULL;
  }

  heapblk_t* region = first_fit(size);
  if (region == NULL) {
    char* next = DATA_START(heap_head + heap_head->size);
    heap_head->next = (heapblk_t*)next;
    region = heap_head->next;
    region->next = NULL;
    region->type = HEAPBLK_USED;
    region->size = size;
    heap_head = region;
    heap_tail = heap_head;
  }

  bytes_allocated += size;

  if (HEAP_DEBUG) {
    PRINTK_SERIAL("[%s]: Allocated %d bytes, now %dKiB used.\n", MODULE_NAME, region->size, bytes_allocated / 1024);
  }
  
  return DATA_START(region);
}

void kfree(void* ptr) {
  heapblk_t* region = ptr - sizeof(heapblk_t);

  for (heapblk_t* block = region; (block != NULL) && (DATA_START(block) != ptr); block = block->next) {
    block->type = HEAPBLK_FREE;
  }

  heap_tail = region;
  bytes_allocated -= region->size;

  if (HEAP_DEBUG) {
    PRINTK_SERIAL("[%s]: Freed %d bytes, now %dKiB used.\n", MODULE_NAME, region->size, bytes_allocated / 1024);
  }
}


void* krealloc(void* oldptr, size_t new_size) {
  heapblk_t* region = (heapblk_t*)(oldptr - sizeof(heapblk_t));
  uint8_t* _new = kmalloc(new_size);
  kmemcpy(_new, oldptr, region->size);
  kfree(oldptr);
  return _new;
}

void heap_init(void) {
  heap_head = k_mmap(NULL, HEAP_PAGES, PROT_READ | PROT_WRITE | PROT_EXEC);
  heap_head->size = 0;
  heap_head->next = NULL;
  heap_head->type = HEAPBLK_USED;
  heap_tail = heap_head;
}
