#include <arch/x86/apic/ioapic.h>
#include <lib/log.h>

// Memory mapped register for I/O APIC access.
#define IOREGSEL                        0x00
#define IOWIN                           0x10

// Other I/O APIC registers.
#define IOAPICID                        0x00
#define IOAPICVER                       0x01
#define IOAPICARB                       0x02
#define IOREDTBL                        0x10

extern void* ioapic_base;

static uint32_t read(uint8_t reg) {
  // Select the register by setting IOREGSEL to reg.
  *(volatile uint32_t*)ioapic_base = reg;

  // Return value in register by reading IOWIN.
  return *(volatile uint32_t*)(ioapic_base + IOWIN);
}

static void write(uint8_t reg, uint32_t value) {
  *(volatile uint32_t*)ioapic_base = reg;
  *(volatile uint32_t*)(ioapic_base + IOWIN) = value;
}

void ioapic_set_entry(uint8_t index, uint64_t data) {
  write(IOREDTBL + index * 2, (uint32_t)data);
  write(IOREDTBL + index * 2 + 1, (uint32_t)(data >> 32));
}

void ioapic_init(void) {
  // Get the max amount of redirection entries from
  // bits 16:23 of IOAPICVER.
  uint32_t max_red_entries = ((read(IOAPICVER) >> 16) & 0xFF) + 1;

  // Mask all redirection entries.
  for (uint32_t i = 0; i < max_red_entries; ++i) {
    ioapic_set_entry(i, 1 << 16);
  }

  printk("[ioapic]: All redirection table entries masked.\n");
}
