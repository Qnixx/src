/*
 * Description: Physical memory manager.
 * Author(s): Ian Marco Moffett
 *
 */


#include <mm/pmm.h>
#include <mm/vmm.h>
#include <lib/limine.h>
#include <lib/log.h>
#include <lib/math.h>
#include <lib/string.h>

#define PMM_DEBUG 0


static volatile struct limine_memmap_request mmap_req = {
  .id = LIMINE_MEMMAP_REQUEST,
  .revision = 0
};

static struct limine_memmap_response* mmap_resp = NULL;
static uint8_t* bitmap = NULL;

#if defined(__x86_64__)

static inline void bitmap_set_bit(size_t bit) {
  bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void bitmap_unset_bit(size_t bit) {
  bitmap[bit / 8] &= ~(1 << (bit % 8));
}


static inline uint8_t bitmap_test(size_t bit) {
  return bitmap[bit / 8] & (1 << (bit % 8));
}

static struct limine_memmap_entry* find_highest_mem_chunk(void) {
  struct limine_memmap_entry* chunk = NULL;

  for (size_t i = 0; i < mmap_resp->entry_count; ++i) {
    struct limine_memmap_entry* entry = mmap_resp->entries[i];
    if (entry->type != LIMINE_MEMMAP_USABLE) continue;

    if (PMM_DEBUG) printk("[pmm]: Found usable memory chunk @%x\n", entry->base);
    if (chunk == NULL) {
      chunk = entry;
    } else if (entry->base > chunk->base) {
      chunk = entry;
    }
  }

  if (PMM_DEBUG) printk("[pmm]: Found highest memory chunk @%x\n", chunk->base);
  return chunk;
}

static inline size_t get_bitmap_size(void) {
  struct limine_memmap_entry* highest_entry = find_highest_mem_chunk();
  size_t highest_address = highest_entry->base + highest_entry->length;
  size_t bitmap_size = ALIGN_UP((highest_address/0x1000)/8, 0x1000);
  return bitmap_size;
}

static void init_bitmap(void) {
  /* Compute the bitmap size */
  printk("[pmm]: Computing PMM bitmap size..\n");
  size_t bitmap_size = get_bitmap_size();
  printk("[pmm]: Bitmap size => %d bytes\n", bitmap_size);

  /* Find a memory chunk for our bitmap */
  for (size_t i = 0; i < mmap_resp->entry_count; ++i) {
    struct limine_memmap_entry* entry = mmap_resp->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
      bitmap = (uint8_t*)(entry->base + VMM_HIGHER_HALF);
      kmemset(bitmap, 0xFF, bitmap_size);
      entry->length -= bitmap_size;
      entry->base += bitmap_size;
      break;
    }
  }

  printk("[pmm]: Bitmap location => %x\n", bitmap);

  /* Setup bitmap based on memory map */
  for (size_t i = 0; i < mmap_resp->entry_count; ++i) {
    struct limine_memmap_entry* entry = mmap_resp->entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
      for (size_t j = 0; j < entry->length; j += 0x1000) {
        bitmap_unset_bit((entry->base + j)/0x1000);
      }
    }
  }
}


void pmm_init(void) {
  mmap_resp = mmap_req.response;
  init_bitmap();
}


static uintptr_t pmm_alloc_inner(void) {
  for (size_t bit = 0; bit < get_bitmap_size()*8; ++bit) {
    if (!(bitmap_test(bit))) {
      bitmap_set_bit(bit);
      return 0x1000*bit;
    }
  }

  return 0;
}


uintptr_t pmm_alloc(size_t frames) {
  uintptr_t mem = 0;
  for (size_t i = 0; i < frames; ++i) {
    if (mem == 0) mem = pmm_alloc_inner();
    if (mem == 0) return 0;
  }

  return mem;
}


void pmm_free(uintptr_t ptr, size_t frames) {
  for (size_t i = 0; i < frames; ++i) {
    bitmap_unset_bit(ptr/0x1000);
    ptr += 0x1000;
  }
}

#endif  //defined(__x86_64__)
