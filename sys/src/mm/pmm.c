#include <mm/pmm.h>
#include <mm/vmm.h>
#include <lib/module.h>
#include <lib/log.h>
#include <lib/math.h>
#include <lib/string.h>

MODULE("pmm");


static struct limine_memmap_response* resp;
static uint8_t* bitmap = NULL;
volatile struct limine_memmap_request mmap_req = {
  .id = LIMINE_MEMMAP_REQUEST,
  .revision = 0
};


static struct limine_memmap_entry* used_mmap_entry = NULL;


static inline void bitmap_set_bit(size_t bit) {
  bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void bitmap_unset_bit(size_t bit) {
  bitmap[bit / 8] &= ~(1 << (bit % 8));
}


static inline uint8_t bitmap_test(size_t bit) {
  return bitmap[bit / 8] & (1 << (bit % 8));
}


static void find_highest(void) {
  /*
   *  We will locate the highest and largest 
   *  chunk of memory.
   *
   *  But first we must find one usable chunk
   *  that we can compare current chunks
   *  with.
   *
   *
   */
  PRINTK_SERIAL("[%s]: High physical memory chunk requested, searching..\n", MODULE_NAME);

 
  for (size_t i = 0; i < resp->entry_count; ++i) {
    if (resp->entries[i]->type == LIMINE_MEMMAP_USABLE && resp->entries[i]->length > 0) {
      used_mmap_entry = resp->entries[i];
      break;
    }
  }

  for (size_t i = 0; i < resp->entry_count; ++i) {
    if (resp->entries[i]->type == LIMINE_MEMMAP_USABLE && resp->entries[i]->base > used_mmap_entry->base && resp->entries[i]->length > 0) {
      used_mmap_entry = resp->entries[i];
      PRINTK_SERIAL("[%s]: Got better chunk at %x\n", MODULE_NAME, used_mmap_entry->base);
    }
  }

  PRINTK_SERIAL("[%s]: Highest chunk located at %x\n", MODULE_NAME, used_mmap_entry->base);
}


static size_t get_bitmap_size(void) {
  size_t highest_addr = used_mmap_entry->base+used_mmap_entry->length;
  size_t bitmap_size = ALIGN_UP((highest_addr/0x1000)/8, 0x1000);

  return bitmap_size;
}


static void init_bitmap(void) {
  PRINTK_SERIAL("[%s]: Setting up new bitmap..\n", MODULE_NAME);

  /*
   *  Compute the size needed for
   *  the bitmap.
   */

  size_t bitmap_size = get_bitmap_size();

  /*
   *  Now we must find a memory chunk
   *  big enough for our bitmap.
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
   *  Now we want to set our bitmap up
   *  based on the memory map.
   *
   */

  for (size_t i = 0; i < resp->entry_count; ++i) {
    struct limine_memmap_entry* e = resp->entries[i];
    if (e->type == LIMINE_MEMMAP_USABLE && e->length >= bitmap_size) {
      for (size_t j = 0; j < e->length; j += 0x1000) {
        bitmap_unset_bit((e->base + j)/0x1000);
      }
    }
  }

  PRINTK_SERIAL("[%s]: Bitmap is at address %x\n", MODULE_NAME, bitmap);
}


uintptr_t pmm_alloc(void) {
  for (size_t bit = 0; bit < get_bitmap_size()*8; ++bit) {
    if (!(bitmap_test(bit))) {
      bitmap_set_bit(bit);
      return 0x1000*bit;
    }
  }

  return 0;
}



void pmm_free(uintptr_t ptr) {
  bitmap_unset_bit(ptr/0x1000);
}


void pmm_init(void) {
  resp = mmap_req.response;

  /*
   *  Locate the highest 
   *  chunk of physical memory.
   *
   */

  find_highest();
  init_bitmap();
}
