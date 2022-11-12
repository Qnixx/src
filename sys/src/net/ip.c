#include <drivers/net/rtl8139.h>
#include <net/ethernet.h>
#include <net/arp.h>
#include <lib/module.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <lib/log.h>
#include <lib/math.h>

MODULE("ip");

/*
 *  IPv4 protocol functionality.
 *
 */

void ip_send(ipv4_address_t dest_ipv4, ip_protocol_t protocol, uint8_t* payload, int length) {
  mac_address_t dest_mac;
  kmemcpy(dest_mac, arp_resolve(dest_ipv4), sizeof(mac_address_t));

  int size = sizeof(iphdr_t) + length;
  uint8_t* pkt = (uint8_t*)kmalloc(size);

  iphdr_t* hdr = (iphdr_t*)pkt;
  hdr->version_ihl = (IP_VER_4 << 4) | (IP_HDR_LEN << 0);
  hdr->len = BIG_ENDIAN(size);
  hdr->ident = BIG_ENDIAN(1);
  hdr->ttl = 64;
  hdr->protocol = protocol;

  hdr->source = IPv4(192, 168, 1, 166);
  hdr->dest = dest_ipv4;

  kmemcpy((uint8_t*)(pkt + sizeof(iphdr_t)), payload, length);
  ethernet_send(dest_mac, ETHERTYPE_IPV4, pkt, size);
}
