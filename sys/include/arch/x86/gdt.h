#ifndef GDT_H_
#define GDT_H_

#include <lib/types.h>
#include <lib/asm.h>

#define KERNEL_CODE 0x28
#define KERNEL_DATA 0x30
#define USER_CODE   0x38
#define USER_DATA   0x40

struct gdt_descriptor {
  uint16_t limit;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_hi;
} _packed;


struct gdtr {
  uint16_t limit;
  uint64_t base;
} _packed;


void load_gdt(void);


#endif
