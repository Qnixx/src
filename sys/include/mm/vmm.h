#ifndef VMM_H_
#define VMM_H_

#include <lib/limine.h>
#include <lib/types.h>
#include <lib/math.h>

extern volatile struct limine_hhdm_request hhdm_request;

#define VMM_HIGHER_HALF (hhdm_request.response->offset)
#define VMM_PHYS(addr) (vmm_get_phys((void*)get_cr3(), (uintptr_t)addr))
#define VMM_VIRT(addr) ((uintptr_t)addr + VMM_HIGHER_HALF)
#define PTE_ADDR_MASK 0x000FFFFFFFFFF000
#define PTE_PRESENT (1ull << 0)
#define PTE_WRITABLE (1ull << 1)
#define PTE_USER (1ULL << 2)
#define PTE_NX (1ULL << 63)
#define PTE_GET_ADDR(VALUE) ((VALUE) & PTE_ADDR_MASK)
#define PAGE_SIZE 0x1000
#define PAGE_ALIGN_DOWN(addr) (ALIGN_DOWN((uintptr_t)addr, PAGE_SIZE))
#define PAGE_ALIGN_UP(addr) (ALIGN_UP((uintptr_t)addr, PAGE_SIZE))

void vmm_map_page(uintptr_t* pml4, uintptr_t vaddr, uintptr_t phys, uint64_t flags);
void vmm_unmap_page(uintptr_t* pml4, uintptr_t vaddr);
uintptr_t vmm_get_phys(uintptr_t* pml4, uintptr_t vaddr);
uintptr_t get_cr3(void);
void* vmm_alloc_page(void);
uintptr_t mkpml4(void);

#endif
