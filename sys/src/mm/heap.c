#include <mm/vmm.h>
#include <mm/heap.h>
#include <lib/module.h>
#include <lib/log.h>

MODULE("heap");

static heapblk_t* heap_head = NULL;
static heapblk_t* heap_tail = NULL;
static size_t total_size = 0;
static size_t bytes_allocated = 0;

static heapblk_t* first_fit(size_t size) {
  for (heapblk_t* block = heap_tail; block != NULL; block = block->next) {
    if (block->type == HEAPBLK_FREE && block->size >= size) return block;
  }

  return NULL;
}

void* kmalloc(size_t size) {
  if (size == 0) return NULL;
  if (bytes_allocated + size > total_size) {
    if(vmm_alloc_page() == NULL) {
      printk(PRINTK_PANIC "Failed to allocate a new heap page\n");
      return NULL;
    }
	  total_size += 0x1000;
    PRINTK_SERIAL("[%s]: Allocated new heap page.\n", MODULE_NAME);
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
  PRINTK_SERIAL("[%s]: Allocated %d bytes, now %dKiB used.\n", MODULE_NAME, region->size, bytes_allocated / 1024);
  return DATA_START(region);
}

void kfree(void* ptr) {
  heapblk_t* region = ptr - sizeof(heapblk_t);

  for (heapblk_t* block = region; (block != NULL) && (DATA_START(block) != ptr); block = block->next) {
    block->type = HEAPBLK_FREE;
  }

  heap_tail = region;
  bytes_allocated -= region->size;
  PRINTK_SERIAL("[%s]: Freed %d bytes, now %dKiB used.\n", MODULE_NAME, region->size, bytes_allocated / 1024);
}

void heap_init(void) {
  total_size = 0x1000;
  heap_head = (heapblk_t*)vmm_alloc_page();
  heap_head->size = 0;
  heap_head->next = NULL;
  heap_head->type = HEAPBLK_USED;
  heap_tail = heap_head;
}