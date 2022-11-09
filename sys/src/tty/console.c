#include <tty/console.h>
#include <drivers/video/framebuffer.h>
#include <lib/string.h>

#define MAKE_FG_BG(fg, bg) ((uint64_t)fg << 32 | bg)
#define EXTRACT_FG(color) ((uint64_t)color >> 32)
#define EXTRACT_BG(color) (color & 0xFFFFFFFF)

static uint32_t x = 0, y = 0;

static void newline(void) {
  if (y >= framebuffer_get_height() - 5) {
    framebuffer_clear(0x000000);
    y = 0;
    x = 0;
    return;
  }

  y += FONT_HEIGHT+4;
  x = 0;
}


static void putstr(const char* str, uint64_t color) {
  for (size_t i = 0; i < kstrlen(str); ++i) {
    if (str[i] == '\n') {
      newline();
      continue;
    }

    framebuffer_putch(x, y, str[i], EXTRACT_BG(color), EXTRACT_FG(color));
    x += FONT_WIDTH;
  }
}



void console_write(const char* fmt, va_list ap) {
  uint64_t color = MAKE_FG_BG(0x808080, 0x000000);

  // static uint8_t panic = 0;

  if (*fmt == '\\') {
    switch (*(fmt + 1)) {
      case '1':
        break;
    }
  }

  for (const char* ptr = fmt; *ptr; ++ptr) {
    if (*ptr == '%') {
      ++ptr;
      switch (*ptr) {
        case 'd':
          putstr(dec2str(va_arg(ap, uint64_t)), color);
          break;
        case 'x':
          putstr(hex2str(va_arg(ap, uint64_t)), color);
          break;
        case 's':
          putstr(va_arg(ap, char*), color);
          break;
        case 'c':
          putstr((char[2]){va_arg(ap, int), 0}, color);
          break;
      }
    } else {
      putstr((char[2]){*ptr, 0}, color);
    }
  }
}
