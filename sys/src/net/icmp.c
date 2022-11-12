#include <net/icmp.h>
#include <net/ip.h>
#include <lib/module.h>
#include <mm/heap.h>
#include <lib/string.h>

MODULE("icmp");

static uint16_t convert(uint16_t os) {
  uint8_t* s = (uint8_t*)&os;
  return (uint16_t)(s[0] << 8 | s[1]);
}

static uint16_t internet_checksum(void* ptr, size_t count) {
  uint32_t checksum = 0;
  uint16_t* w = (uint16_t*)ptr;

  while (count > 1) {
      checksum += convert(*w++);
      if (checksum & 0x80000000) checksum = (checksum & 0xffff) | (checksum >> 16);
      count -= 2;
  }

  while (checksum >> 16) checksum = (checksum & 0xffff) + (checksum >> 16);
  return convert(~checksum);
}


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
