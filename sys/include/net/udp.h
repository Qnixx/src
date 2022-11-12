#ifndef UDP_H_
#define UDP_H_

#include <lib/types.h>
#include <net/ip.h>


typedef struct {
  uint16_t source_port;
  uint16_t dest_port;
  uint16_t length;            // Length of UDP header and data.
} udp_datagram_header_t;


void udp_send(ipv4_address_t dest_ipv4, uint8_t* payload, size_t length);


#endif
