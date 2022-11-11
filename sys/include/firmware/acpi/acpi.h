#ifndef ACPI_H_
#define ACPI_H_

#include <lib/types.h>


void acpi_init(void);
uint16_t acpi_remap_irq(uint8_t irq);

extern void* ioapic_base;
extern void* lapic_base;


#endif
