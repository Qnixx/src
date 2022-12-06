#include <drivers/timer/pit.h>
#include <arch/x86/io.h>


void pit_set_count(uint16_t count) {
  outb(0x43, 0x34);
  outb(0x40, count & 0xFF);
  outb(0x40, (count >> 8) & 0xFF);
}
