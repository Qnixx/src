#include <net/arp.h>
#include <mm/heap.h>
#include <lib/math.h>
#include <lib/string.h>
#include <drivers/net/rtl8139.h>

#define ARP_HW_ETHERNET 1

void arp_send(ipv4_address_t target_paddr) {
  arp_packet_t* packet = kmalloc(sizeof(arp_packet_t));
  packet->htype = BIG_ENDIAN(ARP_HW_ETHERNET);
  packet->ptype = BIG_ENDIAN(ETHERTYPE_IPV4);
  packet->haddr_len = sizeof(mac_address_t);
  packet->paddr_len = sizeof(ipv4_address_t);
  packet->operation = BIG_ENDIAN(ARP_REQUEST);

  kmemset(packet->target_haddr, 0xFF, sizeof(mac_address_t));
  packet->sender_paddr = IPv4(0, 0, 0, 0);

  kmemzero(packet->sender_haddr, sizeof(mac_address_t));
  packet->target_paddr = target_paddr;

  ethernet_send(packet->target_haddr, (uint8_t*)packet, sizeof(arp_packet_t));
}
