#ifndef LAPIC_H_
#define LAPIC_H_

#include <lib/types.h>

uint32_t lapic_read_id(void);
void lapic_send_eoi(void);
void lapic_init(void);


#endif
