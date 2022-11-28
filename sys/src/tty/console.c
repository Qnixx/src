#include <tty/console.h>
#include <lib/string.h>
#include <lib/types.h>
#include <drivers/video/framebuffer.h>

#define MAKE_FG_BG(fg, bg) ((uint64_t)fg << 32 | bg)
#define EXTRACT_FG(color) ((uint64_t)color >> 32)
#define EXTRACT_BG(color) (color & 0xFFFFFFFF)
#define SCROLL_STEP 5

static uint32_t x = 0, y = 0;

static void newline(void) {
  if (y >= framebuffer_get_height()-2) {
    y = 0;
    x = 0;
    framebuffer_clear_screen(0x000000);
  }

  y += FONT_HEIGHT;
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


void vprintk(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  uint64_t color = MAKE_FG_BG(0x808080, 0x000000);

  for (const char* ptr = fmt; *ptr; ++ptr) {
    if (*ptr == '%') {
      ++ptr;

      switch (*ptr) {
        case 'd':
          putstr(dec2str(va_arg(ap, size_t)), color);
          break;
      }
    } else {
      putstr((char[2]){*ptr, 0}, color);
    }
  }
}
