#ifndef PMM_H_
#define PMM_H_

#include <lib/types.h>


uintptr_t pmm_alloc(void);
void pmm_free(uintptr_t ptr);
void pmm_init(void);


#endif
