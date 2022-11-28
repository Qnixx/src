#include <mm/heap.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <lib/module.h>
#include <lib/math.h>
#include <lib/limine.h>
#include <lib/string.h>
#include <tty/console.h>

MODULE_NAME("heap");
MODULE_DESCRIPTION("Kernel heap allocator");
MODULE_AUTHOR("Quinn Stephens");
MODULE_LICENSE("BSD 3-Clause");


static heapblk_t* heap_head = NULL;
static heapblk_t* heap_tail = NULL;
static size_t bytes_allocated = 0;

void* alloc_pages(size_t pages) {
  if(pages == 0) return NULL;
  void* start;

  for(size_t i = 0; i < pages; i++) {
    void* phys = (void*)pmm_alloc();
    if(phys == NULL) {
      vnprintk("Cannot allocate another page!\n", 0);
      return NULL;
    }
  
    if(i == 0) start = phys + VMM_HIGHER_HALF;
  }

  return start;
}

static heapblk_t* first_fit(size_t size) {
  for (heapblk_t* block = heap_tail; block != NULL; block = block->next) {
    if (block->type == HEAPBLK_FREE && block->size >= size) return block;
  }

  return NULL;
}

void* kmalloc(size_t size) {
  if (size == 0) {
    vnprintk("kmalloc(0) in %s()\n", __func__);
    return NULL;
  }

  if (bytes_allocated + size >= MAX_HEAP_SIZE - 1) {
    vnprintk("Heap full (%d bytes allocated) in %s()\n", bytes_allocated, __func__);
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
    vnprintk("Allocated %d bytes, now %dKiB used.\n", region->size, bytes_allocated / 1024);
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
    vnprintk("Freed %d bytes, now %dKiB used.\n", region->size, bytes_allocated / 1024);
  }
}

void* krealloc(void* oldptr, size_t new_size) {
  heapblk_t* region = (heapblk_t*)(oldptr - sizeof(heapblk_t));
  uint8_t* new = kmalloc(new_size);

  // Funky stuff so we can copy a massive 8 BYTES at a time,
  // Instead of the usual 1. This should make reallocating a lot faster :~)
  size_t blocks = region->size / sizeof(uint64_t);
  size_t bytes = blocks * sizeof(uint64_t);
  kmemcpy64(new, oldptr, blocks);
  kmemcpy(new + bytes, oldptr + bytes, region->size - bytes);

  kfree(oldptr);
  return new;
}

void heap_init(void) {
  heap_head = alloc_pages(HEAP_PAGES);
  heap_head->size = 0;
  heap_head->next = NULL;
  heap_head->type = HEAPBLK_USED;
  heap_tail = heap_head;
}
