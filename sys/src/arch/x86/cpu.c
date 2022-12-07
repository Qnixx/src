#include <arch/x86/cpu.h>
#include <lib/asm.h>

void halt(void) {
  ASMV("hlt");
}


void disable_processor(void) {
  ASMV("cli");
  halt();
}
