#include <arch/x86/apic/ioapic.h>
#include <lib/module.h>

MODULE_NAME("ioapic");
MODULE_DESCRIPTION("I/O APIC module");
MODULE_AUTHOR("Ian Marco Moffett");
MODULE_LICENSE("BSD 3-Clause");

// Memory mapped register for I/O APIC access.
#define IOREGSEL                        0x00
#define IOWIN                           0x10

// Other I/O APIC registers.
#define IOAPICID                        0x00
#define IOAPICVER                       0x01
#define IOAPICARB                       0x02
#define IOREDTBL                        0x10

extern void* ioapic_base;

static inline uint32_t read(uint8_t reg) {
  *(volatile uint32_t*)ioapic_base = reg;               // Select register with IOREGSEL.
  return *(volatile uint32_t*)(ioapic_base + IOWIN);    // Fetch value from IOWIN.
}


static inline void write(uint8_t reg, uint32_t val) {
  *(volatile uint32_t*)ioapic_base = reg;               // Select register with IOREGSEL.
  *(volatile uint32_t*)(ioapic_base + IOWIN) = val;     // Write value to IOWIN.
}

void ioapic_set_entry(uint8_t index, uint64_t data) {
  write(IOREDTBL + index * 2, (uint32_t)data);
  write(IOREDTBL + index * 2 + 1, (uint32_t)(data >> 32));
}

void __ioapic_init(void) {
  /*
   * We must fetch the MAX
   * amount of redirection
   * table entries from bits
   * 16:23 of IOAPICVER.
   */

  uint32_t max_redtbl_entries = ((read(IOAPICVER) >> 16) & 0xFF) + 1;

  // Mask all redirection table entries.
  for (uint32_t i = 0; i < max_redtbl_entries; ++i) {
    ioapic_set_entry(i, 1 << 16);
  }
}
