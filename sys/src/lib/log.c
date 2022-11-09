#include <lib/log.h>
#include <tty/console.h>
#include <drivers/serial.h>
#include <lib/string.h>


static uint8_t do_video_logging = 1;


static void serial_log(const char* fmt, va_list ap) {
  if (*fmt == '\\') {
    fmt += 2;
  }

  for (const char* ptr = fmt; *ptr; ++ptr) {
    if (*ptr == '%') {
      ++ptr;

      switch (*ptr) {
        case 'd':
          serial_write(dec2str(va_arg(ap, uint64_t)));
          break;
        case 'x':
          serial_write(hex2str(va_arg(ap, uint64_t)));
          break;
        case 's':
          serial_write(va_arg(ap, char*));
          break;
        case 'c':
          serial_write((char[2]){va_arg(ap, int), 0});
          break;
      }
    } else {
      serial_write((char[2]){*ptr, 0});
    }
  }
}

void disable_video_logging(void) {
  do_video_logging = 0;
}


void enable_video_logging(void) {
  do_video_logging = 1;
}


void printk(const char* fmt, ...) {
  va_list ptr;
  va_start(ptr, fmt);
  
  if (do_video_logging)
    console_write(fmt, ptr);

  serial_log(fmt, ptr);

  va_end(ptr);
}
