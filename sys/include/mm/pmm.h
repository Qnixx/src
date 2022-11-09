#ifndef PMM_H_
#define PMM_H_

#include <lib/limine.h>
#include <lib/types.h>

/*
 *  Physical memory manager for the
 *  Qnixx kernel.
 *
 */


void pmm_init(void);
uintptr_t pmm_alloc(void);
void pmm_free(uintptr_t ptr);


extern volatile struct limine_memmap_request mmap_req;


#endif
