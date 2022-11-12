#ifndef VMM_H_
#define VMM_H_

#include <lib/limine.h>
#include <lib/types.h>

extern volatile struct limine_hhdm_request hhdm_request;

#define VMM_HIGHER_HALF (hhdm_request.response->offset)
#define PTE_ADDR_MASK 0x000FFFFFFFFFF000
#define PTE_PRESENT (1ull << 0)
#define PTE_WRITABLE (1ull << 1)
#define PTE_USER (1ULL << 2)
#define PTE_NX (1ULL << 63)
#define PTE_GET_ADDR(VALUE) ((VALUE) & PTE_ADDR_MASK)

void vmm_map_page(uintptr_t* pml4, uintptr_t vaddr, uintptr_t phys, uint64_t flags);
uintptr_t get_cr3(void);
void* vmm_alloc_page(void);
uintptr_t mkpml4(void);

#endif
