#include <lib/hashmap.h>

HASHMAP_TYPE(int) a;


uint32_t hashmap_hash(const void* data, size_t length) {
  const uint8_t* data_u8 = data;
  uint32_t result = 0;

  for (size_t i = 0; i < length; ++i) {
    uint32_t c = data_u8[i];
    result = c + (result << 6) + (result << 16) - result;
  }

  return result;
}
