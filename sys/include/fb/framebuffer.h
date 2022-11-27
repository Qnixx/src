#pragma once

#include <stdint.h>
#include <limine.h>

#define RED   0x00FF0000
#define BLUE  0xFF0000FF
#define GREEN 0x0000FF00

void* get_fb_addr();

/* Drawing functions */

// Format for color is 0xBBRRGGFF,
//  However, Green&Red use 0xBBRRGG00.
void plot_pixel(uint8_t* fb_addr, int x, int y, uint32_t color);
