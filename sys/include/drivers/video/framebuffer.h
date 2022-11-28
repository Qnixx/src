#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_


#include <lib/types.h>
#include <lib/font.h>


uint32_t* framebuffer_get_address(void);
void framebuffer_init(void);
uint32_t framebuffer_get_index(uint32_t x, uint32_t y);
void framebuffer_putch(uint32_t x, uint32_t y, char c, uint32_t bg, uint32_t fg);
void framebuffer_putpix(uint32_t x, uint32_t y, uint32_t color);
uint64_t framebuffer_get_pitch(void);
uint64_t framebuffer_get_height(void);
uint64_t framebuffer_get_width(void);

#endif
