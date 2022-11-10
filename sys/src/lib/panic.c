#include <lib/panic.h>
#include <lib/asm.h>

void panic(void) {
  ASMV("cli; hlt");
}
