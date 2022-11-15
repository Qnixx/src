#include <drivers/hdd/ahci.h>
#include <arch/bus/pci.h>
#include <lib/module.h>
#include <lib/log.h>

MODULE("ahci");

#define SATA_CONTROLLER_PCI_CLASS 0x01
#define SATA_CONTROLLER_PCI_SUBCLSS 0x06


static pci_device_t dev;
static HBA_MEM* abar = NULL;



void ahci_init(void)  {
  dev = pci_find_any(SATA_CONTROLLER_PCI_CLASS, SATA_CONTROLLER_PCI_SUBCLSS, -1);

  if (!(dev.valid)) {
    PRINTK_SERIAL("[%s]: No SATA controller attached!\n", MODULE_NAME);
    return;
  }

  PRINTK_SERIAL("[%s]: SATA controller found on PCI bus %d, slot %d\n", MODULE_NAME, dev.bus, dev.slot);
  enable_bus_mastering(dev);
  PRINTK_SERIAL("[%s]: Bus mastering enabled for SATA controller.\n", MODULE_NAME);

  abar = (HBA_MEM*)(uint64_t)dev.bar5;
}
