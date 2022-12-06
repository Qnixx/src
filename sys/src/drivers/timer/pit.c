#include <drivers/timer/pit.h>
#include <arch/x86/io.h>


void pit_set_count(uint16_t count) {
  outb(0x43, 0x34);
  outb(0x40, count & 0xFF);
  outb(0x40, (count >> 8) & 0xFF);
}


uint16_t pit_get_count(void) {
  outb(0x43, 0x00);
  uint8_t lo = inb(0x40);
  uint8_t hi = inb(0x40) >> 8;
  return ((uint16_t)hi << 8) | lo;
}
