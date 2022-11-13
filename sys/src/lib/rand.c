#include <lib/rand.h>

static size_t seed = 20;

void srand(uint8_t _seed) {
  seed = _seed;
}

uint32_t rand(void) {
  seed ^= seed << 13;
  seed ^= seed >> 17;
  seed ^= seed << 5;
  return seed;
}
