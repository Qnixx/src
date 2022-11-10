#ifndef PCI_H_
#define PCI_H_

#include <lib/types.h>


uint16_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

typedef struct {
  uint8_t irq_line;
  uint8_t valid : 1;
  uint32_t bar0;
} pci_device_t;


pci_device_t pci_find(unsigned int vendor_id, unsigned int device_id);


#endif
