#ifndef ETHERNET_H_
#define ETHERNET_H_

#include <lib/types.h>

/*
 *  Ethernet and MAC definitions.
 */

typedef uint8_t mac_address_t[6];

// EtherType
// Standard protocol id values used in many different structures
typedef enum {
	ETHERTYPE_ARP  = 0x0806,
	ETHERTYPE_IPV4 = 0x0800,
	ETHERTYPE_IPV6 = 0x86DD
} ethertype_t;

// Ethernet frame header
typedef struct {
	mac_address_t dest;
	mac_address_t src;
	uint16_t ether_type;
	uint8_t payload[0];
} ethernet_header_t;

void ethernet_send(mac_address_t dest, ethertype_t ethertype, uint8_t* data, int length);

#endif
