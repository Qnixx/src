#ifndef VMM_H_
#define VMM_H_

#include <lib/types.h>
#include <lib/limine.h>

#define VMM_HIGHER_HALF (hhdm_request.response->offset)

extern volatile struct limine_hhdm_request hhdm_request;



#endif
