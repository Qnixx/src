#include <lib/limine.h>
#include <lib/font.h>
#include <lib/string.h>
#include <drivers/video/framebuffer.h>


static volatile struct limine_framebuffer_request framebuffer_req = {
  .id = LIMINE_FRAMEBUFFER_REQUEST,
  .revision = 0
};


static struct limine_framebuffer* framebuffer = NULL;


static struct Font {
  uint32_t width;
  uint32_t height;
  uint16_t* data;
} font = {
  .width = FONT_WIDTH,
  .height = FONT_HEIGHT,
  .data = (uint16_t*)DEFAULT_FONT_DATA
};


static uint32_t framebuffer_get_index(uint32_t x, uint32_t y) {
  return x + y * (framebuffer->pitch/4);
}


void framebuffer_putpix(uint32_t x, uint32_t y, uint32_t color) {
  if (x > framebuffer->width-1 || y > framebuffer->height-1) {
    return;
  }

  ((uint32_t*)framebuffer->address)[framebuffer_get_index(x, y)] = color;
}

void framebuffer_putch(uint32_t x, uint32_t y, char c, uint32_t bg, uint32_t fg) {
  c -= 32;
  for (uint32_t cx = 0; cx < font.width; ++cx) {
    for (uint32_t cy = 0; cy < font.height; ++cy) {
      uint16_t col = (font.data[(uint64_t)c * font.width + cx] >> cy) & 1;
      framebuffer_putpix(x + cx, y + cy, col ? fg : bg);
    }
  }
}


uint32_t framebuffer_get_width(void) {
  return framebuffer->width;
}


uint32_t framebuffer_get_height(void) {
  return framebuffer->height;
}


void framebuffer_clear(uint32_t color) {
  kmemset32((uint32_t*)(framebuffer->address), color, framebuffer->width * framebuffer->height);
}


void framebuffer_init(void) {
  framebuffer = framebuffer_req.response->framebuffers[0];
}
