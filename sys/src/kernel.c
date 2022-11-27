#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <lib/log.h>


#define done() for(;;) __asm__("1: b .");
 
struct limine_memmap_request mm_req = {
  .id = LIMINE_MEMMAP_REQUEST,
  .revision = 1
};

void _start(void) {
  printf("YOOO!\n");
  // We're done, just hang...
  done();
}
