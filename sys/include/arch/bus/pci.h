#ifndef PCI_H_
#define PCI_H_

#include <lib/types.h>


uint16_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

typedef struct {
  uint8_t irq_line;
  uint8_t valid : 1;
  uint32_t bar0;
  uint32_t bar4;
  uint32_t bar5;
  uint8_t bus;
  uint8_t slot;
  uint8_t func;
} pci_device_t;


pci_device_t pci_find(unsigned int vendor_id, unsigned int device_id);
pci_device_t pci_find_any(uint8_t class_code, uint8_t subclass_code, int8_t interface_value);
unsigned int pci_read_vendorid(pci_device_t dev);
unsigned int pci_read_deviceid(pci_device_t dev);
void enable_bus_mastering(pci_device_t dev);


#endif
