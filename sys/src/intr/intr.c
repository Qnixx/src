#include <intr/intr.h>
#include <arch/x64/exceptions.h>


void init_interrupts(void) {
  init_exceptions();
}
