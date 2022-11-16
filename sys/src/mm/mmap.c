#include <mm/mmap.h>
#include <mm/vmm.h>
#include <mm/pmm.h>


static uintptr_t alloc_pages(size_t n_pages) {
  uintptr_t ptr = 0;

  for (size_t i = 0; i < n_pages; ++i) {
    uintptr_t c = (uintptr_t)vmm_alloc_page();

    if (ptr == 0) {
      ptr = c;
    }
  }

  return ptr;
}


/*
 *  Returns non-zero value (holds the amount of pages mapped)
 *  if something goes wrong (so we can unmap them).
 *
 */

static size_t map_chunk(uintptr_t chunk, size_t n_pages, uint64_t flags) {
  size_t mapped_pages = 0;

  for (size_t i = 0; i < n_pages; ++i, chunk += 0x1000, ++mapped_pages) {
    uintptr_t phys = pmm_alloc();

    if (phys == 0) {
      return mapped_pages;
    }

    vmm_map_page((void*)get_cr3(), chunk, phys, flags);
  }

  return 0;
}


static uint64_t get_pte_flags(uint8_t prot) {
  uint64_t pte_flags = PTE_NX;

  if (prot & PROT_READ) pte_flags |= PTE_PRESENT;
  if (prot & PROT_WRITE) pte_flags |= PTE_WRITABLE;
  if (prot & PROT_EXEC) pte_flags &= ~(PTE_NX);
  if (prot & PROT_USER) pte_flags |= PTE_USER;
  
  return pte_flags;
}


static void unmap_range(uintptr_t virt, size_t n_pages) {
  for (size_t i = 0; i < n_pages; ++i, virt += PAGE_SIZE) {
    uintptr_t phys = vmm_get_phys((void*)get_cr3(), virt);
    pmm_free(phys);
    vmm_unmap_page((void*)get_cr3(), virt);
  }
}


uint8_t k_mmap(void* addr, size_t n_pages, uint8_t prot) {
  uint64_t vaddr = addr != NULL ? (uintptr_t)addr : alloc_pages(n_pages);
  uint64_t pte_flags = get_pte_flags(prot);

  size_t mapped_pages = 0;
  if ((mapped_pages = map_chunk(vaddr, n_pages, pte_flags)) != 0) {
    unmap_range(vaddr, mapped_pages);
    return 1;
  }

  return 0;
}


void k_munmap(void* addr, size_t n_pages) {
  unmap_range((uintptr_t)addr, n_pages);
}
