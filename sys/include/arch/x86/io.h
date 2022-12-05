#ifndef IO_H_
#define IO_H_

#include <lib/types.h>

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t data);

void outw(uint16_t port, uint16_t data);
uint16_t inw(uint16_t port);

void outl(uint16_t port, uint32_t data);
uint32_t inl(uint16_t port);

void io_wait();

#endif
