#include <mm/vmm.h>
#include <mm/pmm.h>
#include <lib/asm.h>
#include <lib/assert.h>


volatile struct limine_hhdm_request hhdm_request = {
  .id = LIMINE_HHDM_REQUEST,
  .revision = 0
};


static inline void __tlb_flush_single(vaddr_t ptr) {
  ASMV("invlpg (%0)" :: "r" (ptr) : "memory");
}


static uintptr_t* get_next_level(pagemap_t* pagemap, uint16_t index, uint8_t do_alloc) {
  if (pagemap[index] & PTE_PRESENT) {
    return ((uintptr_t*)pagemap[index]);
  }

  if (!(do_alloc)) {
    return NULL;
  }

  uintptr_t next_level = pmm_alloc();
  ASSERT(next_level != 0, "Failed to allocate frame!\n");

  pagemap[index] = (pagemap_t)next_level | PTE_PRESENT | PTE_WRITABLE;
  return (uintptr_t*)next_level;
}


void vmm_map_page(pagemap_t top_level, vaddr_t vaddr, paddr_t paddr, uint64_t flags) {
  size_t pml4_index = (vaddr & ((size_t)0x1FF << 39)) >> 39;
  size_t pdpt_index = (vaddr & ((size_t)0x1FF << 30)) >> 30;
  size_t pd_index = (vaddr & ((size_t)0x1FF << 21)) >> 21;
  size_t pt_index = (vaddr & ((size_t)0x1FF << 12)) >> 12;

 
  uintptr_t* pdpt = get_next_level((pagemap_t*)top_level, pml4_index, 1);
  uintptr_t* pd = get_next_level(pdpt, pdpt_index, 1);
  uintptr_t* pt = get_next_level(pd, pd_index, 1);
  pt[pt_index] = paddr | flags;
  __tlb_flush_single(vaddr);
}
