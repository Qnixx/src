#include <net/checksum.h>

static uint16_t convert(uint16_t os) {
  uint8_t* s = (uint8_t*)&os;
  return (uint16_t)(s[0] << 8 | s[1]);
}

uint16_t internet_checksum(void* ptr, size_t count) {
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
