#include <lib/log.h>
#include <lib/limine.h>
#include <lib/string.h>


static volatile struct limine_terminal_request term_req = {
  .id = LIMINE_TERMINAL_REQUEST,
  .revision = 0
};


static inline void putstr(const char* str) {
  term_req.response->write(term_req.response->terminals[0], str, kstrlen(str));
}



void printk(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  for (const char* ptr = fmt; *ptr; ++ptr) {
    if (*ptr == '%') {
      ++ptr;

      switch (*ptr) {
        case 'd':
          putstr(dec2str(va_arg(ap, uint64_t)));
          break;
        case 'x':
          putstr(hex2str(va_arg(ap, uint64_t)));
          break;
        case 'X':
          putstr(hex2str(va_arg(ap, uint64_t)) + 2);
          break;
        case 's':
          putstr(va_arg(ap, char*));
          break;
        case 'c':
          putstr((char[2]){va_arg(ap, int), 0});
          break;
      }
    } else {
      putstr((char[2]){*ptr, 0});
    }
  }

  va_end(ap);
}

