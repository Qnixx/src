#ifndef PCI_H_
#define PCI_H_

#include <lib/types.h>


typedef struct {
  uint64_t bars[5];
  uint8_t irq_line;
  uint8_t bus;
  uint8_t slot;
  uint8_t func;
} pci_dev_t;


pci_dev_t* pci_find_any(uint8_t class, uint8_t subclass, int8_t interface);
pci_dev_t* pci_find(uint16_t vendor_id, uint16_t device_id);
void pci_enable_bus_mastering(pci_dev_t* dev);

#endif
