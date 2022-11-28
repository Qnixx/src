#include <mm/vmm.h>
#include <mm/pmm.h>
#include <lib/asm.h>
#include <lib/assert.h>

volatile struct limine_hhdm_request hhdm_request = {
  .id = LIMINE_HHDM_REQUEST,
  .revision = 0
};

static inline void __tlb_flush_single(uintptr_t virt) {
  ASMV("invlpg (%0)" :: "r" (virt) : "memory");
}


static uintptr_t* get_next_level(uintptr_t* top_level, uint64_t index) {
  if (top_level[index] & PTE_PRESENT) {
    return ((uintptr_t*)PTE_GET_ADDR(top_level[index]));
  }

  uintptr_t next_level = pmm_alloc();
  ASSERT(next_level != 0, "Could not allocate frame!\n");
  
  top_level[index] = (uintptr_t)next_level | PTE_PRESENT | PTE_WRITABLE;
  return (uintptr_t*)next_level;
}


void vmm_map_page(uintptr_t* pml4, uintptr_t vaddr, uintptr_t paddr, uint64_t flags) {
  size_t pml4_idx = (vaddr & ((size_t)0x1FF << 39)) >> 39;
  size_t pdpt_idx = (vaddr & ((size_t)0x1FF << 30)) >> 30;
  size_t pd_idx = (vaddr & ((size_t)0x1FF << 21)) >> 21;
  size_t pt_idx = (vaddr & ((size_t)0x1FF << 12)) >> 12;

  /*
   *  Go down the structure
   *  hirearchy until we get
   *  the page table.
   *
   */

  uintptr_t* pdpt = get_next_level(pml4, pml4_idx);
  uintptr_t* pd = get_next_level(pdpt, pdpt_idx);
  uintptr_t* pt = get_next_level(pd, pd_idx);
  pt[pt_idx] = paddr | flags;
  __tlb_flush_single(vaddr);
}


void vmm_unmap_page(uintptr_t* pml4, uintptr_t vaddr) {
  size_t pml4_idx = (vaddr & ((size_t)0x1FF << 39)) >> 39;
  size_t pdpt_idx = (vaddr & ((size_t)0x1FF << 30)) >> 30;
  size_t pd_idx = (vaddr & ((size_t)0x1FF << 21)) >> 21;
  size_t pt_idx = (vaddr & ((size_t)0x1FF << 12)) >> 12;

  /*
   *  Go down the structure
   *  hirearchy until we get
   *  the page table.
   *
   */

  uintptr_t* pdpt = get_next_level(pml4, pml4_idx);
  uintptr_t* pd = get_next_level(pdpt, pdpt_idx);
  uintptr_t* pt = get_next_level(pd, pd_idx);
  pt[pt_idx] = 0;
  __tlb_flush_single(vaddr);
}

uintptr_t get_cr3(void) {
  uintptr_t cr3_val = 0;
  ASMV("mov %%cr3, %0" : "=a" (cr3_val));
  return cr3_val;
}
