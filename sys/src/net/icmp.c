#include <net/icmp.h>
#include <net/ip.h>
#include <net/checksum.h>
#include <lib/module.h>
#include <mm/heap.h>
#include <lib/string.h>

MODULE("icmp");


void icmp_send_msg(ipv4_address_t dest_ipv4, uint8_t type, uint8_t code, uint8_t* payload, size_t length) {
  size_t size = sizeof(icmp_header_t) + length;
  uint8_t* packet = kmalloc(size);
  icmp_header_t* header = (icmp_header_t*)packet;
  header->type = type;
  header->code = code;
  kmemcpy((uint8_t*)(packet + sizeof(icmp_header_t)), payload, length);
  header->checksum = internet_checksum(header, sizeof(icmp_header_t) + length);
  ip_send(dest_ipv4, IP_PROTOCOL_ICMP, packet, size);
  kfree(packet);
}
