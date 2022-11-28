#include <mm/pmm.h>
#include <mm/vmm.h>
#include <lib/module.h>
#include <lib/math.h>
#include <lib/limine.h>
#include <lib/string.h>
#include <tty/console.h>

MODULE_NAME("pmm");
MODULE_DESCRIPTION("Physical memory manager");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");


static volatile struct limine_memmap_request mmap_req = {
  .id = LIMINE_MEMMAP_REQUEST,
  .revision = 0
};

static struct limine_memmap_response* resp;
static struct limine_memmap_entry* used_mmap_entry = NULL;
static uint8_t* bitmap = NULL;


static void find_highest_chunk(void) {
  for (size_t i = 0; i < resp->entry_count; ++i) {
    if (resp->entries[i]->type == LIMINE_MEMMAP_USABLE && resp->entries[i]->length > 0) {
      used_mmap_entry = resp->entries[i];
      break;
    }
  }

  for (size_t i = 0; i < resp->entry_count; ++i) {
    if (resp->entries[i]->type == LIMINE_MEMMAP_USABLE && resp->entries[i]->base > used_mmap_entry->base && resp->entries[i]->length > 0) {
      used_mmap_entry = resp->entries[i];
    }
  }
}


static inline size_t get_bitmap_size(void) {
  size_t highest_addr = used_mmap_entry->base+used_mmap_entry->length;
  size_t bitmap_size = ALIGN_UP((highest_addr / PAGE_SIZE) / 8, PAGE_SIZE);
  return bitmap_size;
}

static inline void bitmap_set_bit(size_t bit) {
  bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void bitmap_unset_bit(size_t bit) {
  bitmap[bit / 8] &= ~(1 << (bit % 8));
}


static inline uint8_t bitmap_test(size_t bit) {
  return bitmap[bit / 8] & (1 << (bit % 8));
}


static void init_bitmap(void) {
  /*
   *  Locate highest and 
   *  largest chunk of memory.
   *
   */

  size_t bitmap_size = get_bitmap_size();

  /*
   *  Locate big enough chunk
   *  for our bitmap.
   *
   */

  for (size_t i = 0; i < resp->entry_count; ++i) {
    struct limine_memmap_entry* e = resp->entries[i];

    if (e->type == LIMINE_MEMMAP_USABLE && e->length >= bitmap_size) {
      bitmap = (uint8_t*)(resp->entries[i]->base + VMM_HIGHER_HALF);
      kmemset(bitmap, 0xFF, bitmap_size);
      e->length -= bitmap_size;
      e->base += bitmap_size;
    }
  }

  /*
   *  Setup bitmap based on memory map.
   *
   */

  for (size_t i = 0; i < resp->entry_count; ++i) {
    struct limine_memmap_entry* e = resp->entries[i];
    if (e->type == LIMINE_MEMMAP_USABLE && e->length >= bitmap_size) {
      for (size_t j = 0; j < e->length; j += PAGE_SIZE) {
        bitmap_unset_bit((e->base + j) / PAGE_SIZE);
      }
    }
  }
}

uintptr_t pmm_alloc(void) {
  for (size_t bit = 0; bit < get_bitmap_size()*8; ++bit) {
    if (!(bitmap_test(bit))) {
      bitmap_set_bit(bit);
      return PAGE_SIZE * bit;
    }
  }

  return 0;
}

void pmm_free(uintptr_t ptr) {
  bitmap_unset_bit(ptr / PAGE_SIZE);
}

void pmm_init(void) {
  resp = mmap_req.response;
  find_highest_chunk();
  init_bitmap();
}
