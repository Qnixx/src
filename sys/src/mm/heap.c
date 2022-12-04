/*
 *  Description: Kernel heap alloctor.
 *  Author(s): Ian Marco Moffett
 *
 */

#include <mm/heap.h>
#include <mm/vmm.h>
#include <sync/spinlock.h>
#include <lib/log.h>
#include <lib/math.h>
#include <lib/string.h>
#include <lib/assert.h>

#define MIN_PAGE_COUNT 16
#define MAX_EXP 32
#define MIN_EXP 8
#define DATA_START(mem_block) (((char*) mem_block) + sizeof(block_tag_t))

#define DEBUG 0

static size_t bytes_allocated = 0;        /* How many bytes are allocated */
static uint8_t is_heap_init = 0;          /* 1 if heap has been set up */
static uint8_t lock = 0;


typedef struct _BlockTag {
  uint8_t is_free : 1;
  size_t size;
  struct _BlockTag* next;
} block_tag_t;

static block_tag_t* tail = NULL;
static block_tag_t* head = NULL;


static block_tag_t* best_fit(size_t size) {
  block_tag_t* best = NULL;

  for (block_tag_t* tag = tail; tag != NULL; tag = tag->next) {
    if (tag->is_free && best == NULL) {
      best = tag;
    } else if (tag->is_free && tag->size > size && tag->size < best->size) {
      best = tag;
    }
  }

  return best;
}


static void init(void) {
  head = (void*)vmm_alloc(HEAP_SIZE_PAGES);
  tail = head;
  kmemzero(tail, HEAP_SIZE_PAGES*PAGE_SIZE);
}


void* kmalloc(size_t size) {  
  SPINLOCK_ACQUIRE(lock);

  if (!(is_heap_init)) {
    init();
    is_heap_init = 1;
  }

  /* Check if there is enough memory */
  if (bytes_allocated + size > HEAP_SIZE_PAGES*PAGE_SIZE) {
    SPINLOCK_RELEASE(lock);
    return NULL;
  }
  
  block_tag_t* region = best_fit(size);

  if (region == NULL) {
    char* next = DATA_START(head + head->size);
    ASSERT(head != NULL, "head is NULL");

    head->next = (block_tag_t*)next;
    head = head->next;
    region = head;
    region->next = NULL;
    region->is_free = 0;
    region->size = size;
    tail = head;
  }

  bytes_allocated += size;

  if (DEBUG) {
    printk("[heap]: Allocated %d bytes, %d KiB used.\n", size, bytes_allocated/1024);
  }
  
  SPINLOCK_RELEASE(lock);
  return DATA_START(region);
}


void kfree(void* ptr) {
  block_tag_t* region = ptr - sizeof(block_tag_t);

  for (block_tag_t* block = region; (block != NULL) && (DATA_START(block) != ptr); block = block->next) {
    block->is_free = 1;
  }

  tail = region;
  bytes_allocated -= region->size;
}


void* krealloc(void* oldptr, size_t sz) {
  void* new = kmalloc(sz);
  kmemcpy(new, oldptr, sz);
  kfree(oldptr);
  return new;
}
