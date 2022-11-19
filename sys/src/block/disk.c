#include <block/disk.h>
#include <drivers/storage/ahci.h>
#include <lib/panic.h>
#include <lib/log.h>
#include <lib/assert.h>

static dev_type_t supported_types[] = {
  DEV_BLOCK_SATA,
};

static dev_type_t verify_drive(dev_driver_t* driver_desc) {
  dev_type_t device_type;

  switch (driver_desc->connection) {
    case DEV_CONNECTION_PCI:
      {
        pci_dev_descriptor_t* pci_desc = driver_desc->connection_data;
        ASSERT(pci_desc->device_class == DEV_BLOCK, "BUG: Device is not a block device! (please tell us about this issue).\n");
        device_type = pci_desc->device_type;
      }

      break;
    default:
      printk(PRINTK_PANIC "BUG: Block device is not on PCI bus (please tell us about this issue)\n");
      panic();
      break;
  }

  for (size_t i = 0; i < sizeof(supported_types); ++i) {
    if (device_type == supported_types[i]) {
      return device_type;
    }
  }

  printk(PRINTK_PANIC, "BUG: Found unsupported block device (please tell us about this issue)\n");
  panic();
  __builtin_unreachable();
}

void disk_read_lba(dev_driver_t* driver_desc, uint64_t lba, uint32_t sector_count, uint16_t* buf) {
  dev_type_t type = verify_drive(driver_desc);

  switch (type) {
    case DEV_BLOCK_SATA:
      sata_read_drive(IFACE_2_SATA_DEV(driver_desc->ifaces), lba, sector_count, buf);
      break;
  }
}
