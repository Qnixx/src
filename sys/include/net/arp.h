#ifndef ARP_H_
#define ARP_H_

#include <net/ip.h>
#include <net/ethernet.h>
#include <lib/asm.h>
#include <lib/types.h>

#define ARP_REQUEST 1
#define ARP_RESPONSE 2

typedef struct {
    uint16_t htype;
    uint16_t ptype;
    uint8_t haddr_len;
    uint8_t paddr_len;
    uint16_t operation;

    mac_address_t sender_haddr;
    ipv4_address_t sender_paddr;
    mac_address_t target_haddr;
    ipv4_address_t target_paddr;
} _packed arp_packet_t;

void arp_send(ipv4_address_t target_paddr);

#endif
