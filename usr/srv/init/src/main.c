#include <sys/syscall.h>

void main(void) {
  const char* msg = "[init]: Coming soon..\n";
  syscall(0x0, msg);
  while (1);
}
