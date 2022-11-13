#ifndef SMP_H_
#define SMP_H_

#include <lib/types.h>
#include <lib/limine.h>
#include <proc/proc.h>

void smp_init(core_t** core_list_ptr);
size_t smp_get_core_count(void);
size_t smp_get_bsp_id(void);


#endif
