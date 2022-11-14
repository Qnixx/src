#ifndef IF_H_
#define IF_H_

#include <lib/types.h>


typedef struct {
  const char* idstr;      // If you put together id and idstr you could for example get eth0.
  uint16_t id;
} net_interface_t;


void if_manager_init(void);
uint8_t interface_exists(const char* idstr, uint16_t id);
uint16_t create_interface(const char* idstr);

#endif
