#include <drivers/usb/xhci.h>
#include <arch/bus/pci.h>
#include <dev/dev.h>
#include <lib/module.h>
#include <lib/log.h>

MODULE("xhci");

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


static pci_device_t dev;


void xhci_init(void) {
  dev = pci_find_any(XHCI_PCI_CLASS, XHCI_PCI_SUBCLASS, XHCI_PCI_INTERFACE);

  if (!(dev.valid)) {
    PRINTK_SERIAL("[%s]: Could not find an XHCI controller.\n", MODULE_NAME);
    return;
  }

  PRINTK_SERIAL("[%s]: XHCI controller found on PCI bus %d, slot %d\n", MODULE_NAME, dev.bus, dev.slot);

}
