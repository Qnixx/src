#include <arch/bus/pci.h>
#include <arch/x86/io.h>

#define CONFIG_ADDR 0xCF8
#define CONFIG_DATA 0xCFC


uint16_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
  uint32_t lbus = (uint32_t)bus;
  uint32_t lslot = (uint32_t)slot;
  uint32_t lfunc = (uint32_t)func;
  uint16_t tmp = 0;

  uint32_t address =  (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
  outl(CONFIG_ADDR, address);
  tmp = (uint16_t)((inl(CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
  return tmp;
}


static inline uint8_t read_irq_line(uint8_t bus, uint8_t slot, uint8_t func) {
  return pci_config_read(bus, slot, func, 0x3C) & 0xFF;
}


static inline uint8_t pci_read_vendor(uint8_t bus, uint8_t slot, uint8_t func) {
  return pci_config_read(bus, slot, func, 0x0);
}


static inline uint8_t pci_read_devid(uint8_t bus, uint8_t slot, uint8_t func) {
  return pci_config_read(bus, slot, func, 0x2);
}

static inline uint32_t get_bar0(uint8_t bus, uint8_t slot, uint8_t func) {
  uint16_t lo = pci_config_read(bus, slot, func, 0x10);
  uint16_t hi = pci_config_read(bus, slot, func, 0x12);
  return ((uint32_t)hi << 16 | lo);
}


pci_device_t pci_find(unsigned int vendor_id, unsigned int device_id) {
  pci_device_t dev;

  for (uint8_t bus = 0; bus < 5; ++bus) {
    for (uint8_t slot = 0; slot < 32; ++slot) {
      for (uint8_t func = 0; func < 8; ++func) {
        if (pci_read_devid(bus, slot, func) == device_id && pci_read_vendor(bus, slot, func) == vendor_id) {
          dev.irq_line = read_irq_line(bus, slot, func);
          dev.valid = 1;
          dev.bar0 = get_bar0(bus, slot, func);
          return dev;
        }
      }
    }
  }

  dev.valid = 0;
  return dev;
}
