#ifndef LAPIC_H_
#define LAPIC_H_

#include <lib/types.h>


void lapic_send_ipi(uint8_t apic_id, uint8_t vector);
void lapic_send_init(uint8_t apic_id);
void lapic_send_startup(uint8_t apic_id, uint8_t vector);
uint32_t lapic_read_id(void);
void lapic_send_eoi(void);
void lapic_init(void);

#endif
