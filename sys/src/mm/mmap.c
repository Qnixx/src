#include <mm/mmap.h>
#include <mm/pmm.h>
#include <mm/vmm.h>


static uintptr_t alloc_frames(size_t frame_count) {
  uintptr_t paddr = 0;
  for (size_t i = 0; i < frame_count; ++i) {
    uintptr_t current_paddr = pmm_alloc();

    if (paddr == 0) {
      paddr = current_paddr;
    }
  }

  return paddr;
}


static size_t get_pte_flags(uint8_t prot) {
  size_t pte_flags = 0;

  if (prot & PROT_READ) pte_flags |= PTE_PRESENT;
  if (prot & PROT_WRITE) pte_flags |= PTE_WRITABLE;
  if (prot & PROT_USER) pte_flags |= PTE_USER;
  return pte_flags;
}


void k_mmap(void* vaddr, size_t n_pages, uint8_t prot) {
  uintptr_t frame_base = alloc_frames(n_pages);

  while (n_pages) {
    vmm_map_page((void*)get_cr3(), (uintptr_t)vaddr, frame_base, get_pte_flags(prot));
    frame_base += PAGE_SIZE;
    vaddr += PAGE_SIZE;
    --n_pages;
  }
}
