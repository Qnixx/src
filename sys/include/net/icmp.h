#ifndef ICMP_H_
#define ICMP_H_

#include <lib/types.h>
#include <lib/asm.h>
#include <net/ip.h>

typedef struct {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
} _packed icmp_header_t;

void icmp_send_msg(ipv4_address_t dest_ipv4, uint8_t type, uint8_t code, uint8_t* payload, size_t length);

#endif
