#ifndef VMM_H_
#define VMM_H_

#include <lib/types.h>
#include <lib/limine.h>

#define VMM_HIGHER_HALF (hhdm_request.response->offset)

extern volatile struct limine_hhdm_request hhdm_request;


typedef uintptr_t pagemap_t;
typedef uintptr_t vaddr_t;
typedef uintptr_t paddr_t;

#define PTE_ADDR_MASK 0x000FFFFFFFFFF000
#define PTE_PRESENT (1ull << 0)
#define PTE_WRITABLE (1ull << 1)
#define PTE_USER (1ULL << 2)
#define PTE_NX (1ULL << 63)
#define PTE_GET_ADDR(VALUE) ((VALUE) & PTE_ADDR_MASK)
#define PAGE_SIZE 0x1000


void vmm_map_page(pagemap_t top_level, vaddr_t vaddr, paddr_t paddr, uint64_t flags);

#endif
