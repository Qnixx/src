#include <drivers/net/rtl8139.h>
#include <net/ethernet.h>
#include <net/arp.h>
#include <net/checksum.h>
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
  mac_address_t* mac_addr = arp_resolve(dest_ipv4);

  if (mac_addr == NULL) {
    PRINTK_SERIAL(
        "[%s]: Could not resolve: %d.%d.%d.%d\n", MODULE_NAME,
                  (dest_ipv4 >> 0) & 0xFF, (dest_ipv4 >> 8) & 0xFF, (dest_ipv4 >> 16) & 0xFF, (dest_ipv4 >> 24) & 0xFF
                  );
    return;
  }
  kmemcpy(dest_mac, mac_addr, sizeof(mac_address_t));

  int size = sizeof(iphdr_t) + length;
  uint8_t* pkt = (uint8_t*)kmalloc(size);

  iphdr_t* hdr = (iphdr_t*)pkt;
  hdr->version_ihl = (IP_VER_4 << 4) | (IP_HDR_LEN << 0);
  hdr->len = BIG_ENDIAN(size);
  hdr->ident = BIG_ENDIAN(1);
  hdr->ttl = 64;
  hdr->protocol = protocol;
  hdr->checksum = internet_checksum(hdr, sizeof(iphdr_t) + length);

  hdr->source = IPv4(192, 168, 1, 166);
  hdr->dest = dest_ipv4;

  kmemcpy((uint8_t*)(pkt + sizeof(iphdr_t)), payload, length);
  ethernet_send(dest_mac, ETHERTYPE_IPV4, pkt, size);
}
