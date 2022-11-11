#include <net/arp.h>
#include <mm/heap.h>
#include <lib/math.h>
#include <lib/module.h>
#include <lib/string.h>
#include <lib/log.h>
#include <drivers/net/rtl8139.h>

#define ARP_HW_ETHERNET 1

MODULE("arp");

uint16_t htons(uint16_t v) {
  return (v >> 8) | (v << 8);
}

uint32_t htonl(uint32_t v) {
  return htons(v >> 16) | (htons((uint16_t) v) << 16);
}


void arp_send(ipv4_address_t target_paddr) {
  // Allocate memory for a packet.
  arp_packet_t* packet = kmalloc(sizeof(arp_packet_t));

  // Set header.
  packet->htype = BIG_ENDIAN(ARP_HW_ETHERNET);
  packet->ptype = BIG_ENDIAN(ETHERTYPE_IPV4);
  packet->haddr_len = sizeof(mac_address_t);
  packet->paddr_len = sizeof(ipv4_address_t);
  packet->operation = BIG_ENDIAN(ARP_REQUEST);

  // Target hardware address if a brodcast address (FF:FF..)
  kmemset(packet->target_haddr, 0xFF, sizeof(mac_address_t));
  packet->sender_paddr = IPv4(0, 0, 0, 0);
  
  kmemcpy(packet->sender_haddr, rtl8139_mac_addr, sizeof(mac_address_t));
  packet->target_paddr = target_paddr;

  ethernet_send(packet->target_haddr, (uint8_t*)packet, sizeof(arp_packet_t));

  ssize_t spin = 10000000;

  // Wait until a packet arrives.
  while (!(rtl8139_got_packet()) && spin) {
    --spin;
  }

  if (spin > 0) {
    // Skip ethernet header.
    arp_packet_t* pkt = rtl8139_read_packet() + sizeof(ethernet_header_t);
  
    // Write out response.
    if (pkt->operation == BIG_ENDIAN(ARP_RESPONSE)) {
      PRINTK_SERIAL("[%s]: Got ARP response from %X:%X:%X:%X:%X:%X.\n", MODULE_NAME,
          pkt->sender_haddr[0], pkt->sender_haddr[1], pkt->sender_haddr[2], pkt->sender_haddr[3], pkt->sender_haddr[4], pkt->sender_haddr[5]);
    }
  }
}
