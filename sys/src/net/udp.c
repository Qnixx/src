#include <net/udp.h>
#include <mm/heap.h>
#include <lib/string.h>
#include <lib/log.h>
#include <lib/math.h>


void udp_send(ipv4_address_t dest_ipv4, uint8_t* payload, size_t length) {
  size_t size = sizeof(udp_datagram_header_t) + length;
  uint8_t* packet = kmalloc(size);

  udp_datagram_header_t* hdr = (udp_datagram_header_t*)packet;
  hdr->source_port = BIG_ENDIAN(11);
  hdr->dest_port = BIG_ENDIAN(11);
  hdr->length = BIG_ENDIAN(size);
  kmemcpy((uint8_t*)(packet + sizeof(udp_datagram_header_t)), payload, length);
  
  ip_send(dest_ipv4, IP_PROTOCOL_UDP, packet, size);
  kfree(packet);
}
