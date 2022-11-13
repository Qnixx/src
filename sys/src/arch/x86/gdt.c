#include <arch/x86/gdt.h>
#include <lib/asm.h>
#include <lib/log.h>
#include <lib/module.h>

MODULE("gdt");

static struct gdt_descriptor_t gdt[] = {
  // 0x00 (NULL)
  {0}, 

  // 0x08
  {
      .limit       = 0xFFFF,
      .base_low    = 0x0000,
      .base_mid    = 0x00,
      .access      = 0b10011010,
      .granularity = 0b00000000,
      .base_hi     = 0x00
  },

  // 0x10
  {
    .limit       = 0xFFFF,
    .base_low    = 0x0000,
    .base_mid    = 0x00,
    .access      = 0b10010010,
    .granularity = 0b00000000,
    .base_hi     = 0x00
  },

  // 0x18
  {
    .limit       = 0xFFFF,
    .base_low    = 0x0000,
    .base_mid    = 0x00,
    .access      = 0b10011010,
    .granularity = 0b11001111,
    .base_hi     = 0x00
  },

  // 0x20
  {
    .limit       = 0xFFFF,
    .base_low    = 0x0000,
    .base_mid    = 0x00,
    .access      = 0b10010010,
    .granularity = 0b11001111,
    .base_hi     = 0x00
  },

  // 0x28 (Kernel code)
  {
    .limit       = 0x0000,
    .base_low    = 0x0000,
    .base_mid    = 0x00,
    .access      = 0b10011010,
    .granularity = 0b00100000,
    .base_hi     = 0x00
  }, 

  // 0x30 (Kernel data)
  {
    .limit       = 0x0000,
    .base_low    = 0x0000,
    .base_mid    = 0x00,
    .access      = 0b10010010,
    .granularity = 0b00000000,
    .base_hi     = 0x00
  },

  // 0x38 (User code)
  {
    .limit       = 0x0000,
    .base_low    = 0x0000,
    .base_mid    = 0x00,
    .access      = 0b11111010,
    .granularity = 0b10101111,
    .base_hi     = 0x00
  },

  // 0x40 (User data)
  {
    .limit       = 0x0000,
    .base_low    = 0x0000,
    .base_mid    = 0x00,
    .access      = 0b11110010,
    .granularity = 0b00000000,
    .base_hi     = 0x00
  },

  // 0x9.
  {0}, {0}, {0}
};


static struct gdtr_t gdtr = {
  sizeof(gdt) - 1,
  (uintptr_t)gdt
};


void load_gdt(void) {
  ASMV("lgdt %0" :: "m" (gdtr));
}
