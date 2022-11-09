#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

#include <lib/types.h>


void framebuffer_putpix(uint32_t x, uint32_t y, uint32_t color);
void framebuffer_putch(uint32_t x, uint32_t y, char c, uint32_t bg, uint32_t fg);
void framebuffer_init(void);
uint32_t framebuffer_get_width(void);
uint32_t framebuffer_get_height(void);
void framebuffer_clear(uint32_t color);

#endif
