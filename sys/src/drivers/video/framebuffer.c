#include <drivers/video/framebuffer.h>
#include <lib/limine.h>


static volatile struct limine_framebuffer_request framebuffer_req = {
  .id = LIMINE_FRAMEBUFFER_REQUEST,
  .revision = 0
};


void framebuffer_init(void) {}
