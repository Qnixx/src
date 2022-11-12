#ifndef NET_CHECKSUM_H_
#define NET_CHECKSUM_H_

#include <lib/types.h>

uint16_t internet_checksum(void* ptr, size_t count);


#endif
