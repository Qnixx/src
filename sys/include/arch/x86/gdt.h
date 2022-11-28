#ifndef GDT_H
#define GDT_H

#include <lib/types.h>

#define KERNEL_CODE 0x28
#define KERNEL_DATA 0x30

struct GDTR {
  uint16_t limit;
  uint64_t ptr;
} __attribute__((packed));


struct GDTDesc {
  uint16_t limit;
  uint16_t base_low;
  uint8_t  base_mid;
  uint8_t  access;
  uint8_t  granularity;
  uint8_t  base_hi;
} __attribute__((packed));

void load_gdt(void);

#endif
