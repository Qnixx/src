#ifndef VMM_H_
#define VMM_H_

#include <lib/limine.h>

extern volatile struct limine_hhdm_request hhdm_request;

#define VMM_HIGHER_HALF (hhdm_request.response->offset)


#endif
