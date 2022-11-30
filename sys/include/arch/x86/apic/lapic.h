#ifndef LAPIC_H_
#define LAPIC_H_

#include <lib/types.h>

void __lapic_init(void);
void lapic_send_eoi(void);
void lapic_send_ipi(uint8_t apic_id, uint8_t vector);
uint8_t lapic_read_id(void);

#endif // LAPIC_H_
