#ifndef PMM_H_
#define PMM_H_

#include <lib/types.h>


void pmm_init(void);
uintptr_t pmm_alloc(size_t frames);
void pmm_free(uintptr_t ptr, size_t frames);


#endif
