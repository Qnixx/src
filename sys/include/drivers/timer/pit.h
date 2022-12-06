#ifndef PIT_H_
#define PIT_H_

#include <lib/types.h>

#define PIT_DIVIDEND 1193180


void pit_set_count(uint16_t count);
uint16_t pit_get_count(void);



#endif
