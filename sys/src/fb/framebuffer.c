#include "fb/framebuffer.h"

static volatile struct limine_framebuffer_request fb = {
  .id = LIMINE_FRAMEBUFFER_REQUEST,
  .revision = 0
};


void* get_fb_addr(){
  return fb.response->framebuffers[0]->address;
}

void plot_pixel(uint8_t* fb_addr, int x, int y, uint32_t color){
  uint64_t pitch   = fb.response->framebuffers[0]->pitch;
  uint64_t pix_wid = fb.response->framebuffers[0]->width;
  unsigned where = x*pix_wid + y*pitch;

  fb_addr[where] = color & 255;              // BLUE
  fb_addr[where + 1] = (color >> 8) & 255;   // GREEN
  fb_addr[where + 2] = (color >> 16) & 255;  // RED
}