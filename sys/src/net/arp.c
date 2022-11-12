#include <net/arp.h>
#include <mm/heap.h>
#include <lib/math.h>
#include <lib/module.h>
#include <lib/string.h>
#include <lib/log.h>
#include <drivers/net/rtl8139.h>

#define ARP_HW_ETHERNET 1

MODULE("arp");


mac_address_t* arp_resolve(ipv4_address_t target_paddr) {
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
  
  // Copy the network cards MAC address into the packet sender_haddr field.
  kmemcpy(packet->sender_haddr, rtl8139_mac_addr, sizeof(mac_address_t));
  packet->target_paddr = target_paddr;
  
  // Send it off!
  ethernet_send(packet->target_haddr, ETHERTYPE_ARP, (uint8_t*)packet, sizeof(arp_packet_t));
  kfree(packet);

  ssize_t spin = 10000000;

  // Wait until a packet arrives.
  while (!(rtl8139_got_packet()) && spin) {
    --spin;
  }

  if (spin <= 0) return NULL;

  // Skip ethernet header.
  arp_packet_t* pkt = rtl8139_read_packet() + sizeof(ethernet_header_t);
  
  // Write out reply
  if (pkt->operation == BIG_ENDIAN(ARP_REPLY)) {
    PRINTK_SERIAL(
      "[%s]: Got ARP reply; %d.%d.%d.%d is at %X:%X:%X:%X:%X:%X.\n", MODULE_NAME,
      (pkt->sender_paddr >> 0) & 0xFF, (pkt->sender_paddr >> 8) & 0xFF, (pkt->sender_paddr >> 16) & 0xFF, (pkt->sender_paddr >> 24) & 0xFF,
      pkt->sender_haddr[0], pkt->sender_haddr[1], pkt->sender_haddr[2], pkt->sender_haddr[3], pkt->sender_haddr[4], pkt->sender_haddr[5]
    );
  } else {
    return NULL;
  }

  return &pkt->sender_haddr;
}
