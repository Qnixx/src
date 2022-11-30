#include <drivers/timer/pit.h>
#include <arch/x86/io.h>


void init_pit(void) {
  int divisor = 1193180/PIT_FREQ;
  outb(0x43, 0x36);
  outb(0x40, divisor & 0xFF);
  outb(0x40, divisor >> 8);
}
