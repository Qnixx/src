#include <drivers/usb/xhci.h>
#include <arch/bus/pci.h>
#include <dev/dev.h>

#define XHCI_PCI_CLASS 0x0C
#define XHCI_PCI_SUBCLASS 0x03
#define XHCI_PCI_INTERFACE 0x30

static pci_dev_descriptor_t pci_desc = {
  .device_class = DEV_BLOCK,
  .device_type = DEV_BLOCK_USB
};

static dev_driver_t xhci_generic = {
  .name = "XHCI",
  .ioctl = NULL,
  .ifaces = NULL,
  .iface_count = 0,
  .connection = DEV_CONNECTION_PCI,
  .connection_data = &pci_desc
};


void xhci_init(void) {
  driver_init(&xhci_generic);
}
