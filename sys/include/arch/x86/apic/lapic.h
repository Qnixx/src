#ifndef LAPIC_H_
#define LAPIC_H_

#include <lib/types.h>

void __lapic_init(void);
void lapic_send_eoi(void);

#endif // LAPIC_H_
