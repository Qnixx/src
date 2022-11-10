#ifndef PIC_H_
#define PIC_H_


#include <lib/types.h>


void pic_init(void);
void pic_enable(uint8_t vector);
void pic_disable(uint8_t vector);
void pic_EOI(uint8_t vector);
uint16_t pic_getIRR(void);
void pic_init(void);

#endif
