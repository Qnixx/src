#include <lib/log.h>
#include <tty/console.h>


void printk(const char* fmt, ...) {
  va_list ptr;
  va_start(ptr, fmt);

  console_write(fmt, ptr);

  va_end(ptr);
}
