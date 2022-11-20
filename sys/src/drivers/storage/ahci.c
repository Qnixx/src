#include <drivers/storage/ahci.h>
#include <drivers/timer/pit.h>
#include <arch/bus/pci.h>
#include <lib/module.h>
#include <lib/log.h>
#include <lib/string.h>
#include <lib/math.h>
#include <lib/asm.h>
#include <lib/assert.h>
#include <mm/heap.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <dev/dev.h>

#define AHCI_DEBUG 1

MODULE("ahci");

#define SATA_CONTROLLER_PCI_CLASS 0x01
#define SATA_CONTROLLER_PCI_SUBCLSS 0x06

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

#define GHC_AHCI_ENABLE (1 << 31)
#define GHC_HBA_RESET   (1 << 0)

#define HBA_PxCMD_ST    (1 << 0)
#define HBA_PxCMD_FRE   (1 << 4)
#define HBA_PxCMD_FR    (1 << 14)
#define HBA_PxCMD_CR    (1 << 15)

#define HBA_PxIS_TFES      (1 << 30)
#define HBA_PxIS_HBFS      (1 << 29)
#define HBA_PxIS_IFS       (1 << 27)
#define HBA_PxIS_HBDS      (1 << 28)
#define HBA_PxIS_INFS      (1 << 26)
#define HBA_ALL_ERRORS (HBA_PxIS_TFES | HBA_PxIS_HBFS | HBA_PxIS_HBDS | HBA_PxIS_IFS | HBA_PxIS_INFS)

#define SATA_SIG_ATA 0x00000101     // SATA drive
#define SATA_SIG_ATAPI 0xEB140101   // SATAPI drive
#define SATA_SIG_SEMB 0xC33C0101    // Enclosure management bridge
#define SATA_SIG_PM 0x96690101      // Port multiplier.

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4


static pci_dev_descriptor_t pci_dev = {
  .device_class = DEV_BLOCK,
  .device_type = DEV_BLOCK_SATA
};

static dev_driver_t sata_generic = {
  .name = "SATA",
  .connection = DEV_CONNECTION_PCI,
  .iface_count = 0,
  .connection_data = &pci_dev,
  .next = NULL
};

static pci_device_t dev;
static HBA_MEM* abar = NULL;
static uint32_t n_slots = 0;
static size_t drive_count = 0;


static int find_cmdslot(sata_dev_t* device) {
  uint32_t slots = (device->port->sact | device->port->ci);
  for (uint32_t i = 0; i < n_slots; ++i, slots >>= 1) {
    if (!(slots & 1)) {
      return i;
    }
  }

  PRINTK_SERIAL("[%s]: Failed to find free command list entry.\n", MODULE_NAME);
  return -1;
}


static uint32_t check_type(HBA_PORT* p) {
  uint32_t ssts = p->ssts;
  uint8_t ipm = (ssts >> 8) & 0x0F;
  uint8_t det = ssts & 0x0F;

  if (det != HBA_PORT_DET_PRESENT || ipm != HBA_PORT_IPM_ACTIVE) {
    return AHCI_DEV_NULL;
  }

  switch (p->sig) {
    case SATA_SIG_ATAPI:
      return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
      return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
      return AHCI_DEV_PM;
    default:
      return AHCI_DEV_SATA;
  }
}


static void send_cmd(sata_dev_t* device, uint32_t slot) {
  while ((device->port->tfd & 0x88) != 0);
  device->port->cmd &= ~(1 << 0);
  while ((device->port->cmd & HBA_PxCMD_CR) != 0);

  device->port->cmd |= HBA_PxCMD_FR | HBA_PxCMD_ST;
  device->port->ci = 1 << slot;

  while (device->port->ci & (1 << slot) != 0);

  device->port->cmd &= ~(HBA_PxCMD_ST);
  while ((device->port->cmd & HBA_PxCMD_ST) != 0);
  device->port->cmd &= ~(HBA_PxCMD_FRE);
}

static inline volatile HBA_CMD_TBL* set_prdt(sata_dev_t* device, uint64_t buf_phys, uint8_t interrupt, uint32_t byte_count, uint64_t cmdslot) {
  volatile HBA_CMD_TBL* cmdtable = (volatile HBA_CMD_TBL*)(device->ctba_virts[cmdslot]);
  cmdtable->prdt_entry[0].dba = (uint32_t)buf_phys;
  cmdtable->prdt_entry[0].dbau = buf_phys >> 32;
  cmdtable->prdt_entry[0].dbc = byte_count | ((interrupt & 1) << 31);
  return cmdtable;
}

void sata_read_drive(sata_dev_t* device, uint64_t lba, uint32_t sector_count, uint16_t* buf) {
  ASSERT((uint64_t)buf % 4096 == 0, "Buffer not 4K aligned!\n");

  int cmdslot = find_cmdslot(device);
  
  if (cmdslot == -1) {
    PRINTK_SERIAL(PRINTK_RED "[%s]: Could not find a command slot!\n", MODULE_NAME);
    return;
  }

  volatile HBA_CMD_HEADER* cmdheader = (volatile HBA_CMD_HEADER*)(device->cmdlist_virt);
  cmdheader[cmdslot].cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t);
  cmdheader[cmdslot].w = 0;
  cmdheader[cmdslot].p = 0;
  cmdheader[cmdslot].prdtl = 8;

  volatile HBA_CMD_TBL* cmd_table = set_prdt(device, VMM_PHYS(buf), 0, sector_count*512, cmdslot);
  volatile FIS_REG_H2D* fis = (volatile FIS_REG_H2D*)cmd_table->cfis;
  kmemzero((void*)fis, sizeof(FIS_REG_H2D));
  
  fis->command = 0x25;         // Read DMA extended.
  fis->fis_type = 0x27;        // Host to device.
  fis->flags = (1 << 7);
  fis->device = 1 << 6;        // LBA mode.

  fis->lba0 = (uint8_t)(lba & 0xFF);
  fis->lba1 = (uint8_t)((lba >> 8) & 0xFF);
  fis->lba2 = (uint8_t)((lba >> 16) & 0xFF);
  fis->lba3 = (uint8_t)((lba >> 24) & 0xFF);
  fis->lba4 = (uint8_t)((lba >> 32) & 0xFF);
  fis->lba5 = (uint8_t)((lba >> 40) & 0xFF);
  fis->countl = sector_count & 0xFF;
  fis->counth = (sector_count >> 8) & 0xFF;

  send_cmd(device, cmdslot);
  
}

void sata_write_drive(sata_dev_t* device, uint64_t lba, uint32_t sector_count, uint16_t* buf) {
  ASSERT((uint64_t)buf % 4096 == 0, "Buffer not 4K aligned!\n");
  int cmdslot = find_cmdslot(device);
  
  if (cmdslot == -1) {
    PRINTK_SERIAL(PRINTK_RED "[%s]: Could not find a command slot!\n", MODULE_NAME);
    return;
  }

  volatile HBA_CMD_HEADER* cmdheader = (volatile HBA_CMD_HEADER*)(device->cmdlist_virt);
  cmdheader[cmdslot].cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t);
  cmdheader[cmdslot].w = 1;
  cmdheader[cmdslot].p = 0;
  cmdheader[cmdslot].prdtl = 8;

  volatile HBA_CMD_TBL* cmd_table = set_prdt(device, VMM_PHYS(buf), 0, sector_count*512, cmdslot);
  volatile FIS_REG_H2D* fis = (volatile FIS_REG_H2D*)cmd_table->cfis;
  kmemzero((void*)fis, sizeof(FIS_REG_H2D));
  
  fis->command = 0x35;         // Write DMA extended.
  fis->fis_type = 0x27;        // Host to device.
  fis->flags = (1 << 7);
  fis->device = 1 << 6;        // LBA mode.

  fis->lba0 = (uint8_t)(lba & 0xFF);
  fis->lba1 = (uint8_t)((lba >> 8) & 0xFF);
  fis->lba2 = (uint8_t)((lba >> 16) & 0xFF);
  fis->lba3 = (uint8_t)((lba >> 24) & 0xFF);
  fis->lba4 = (uint8_t)((lba >> 32) & 0xFF);
  fis->lba5 = (uint8_t)((lba >> 40) & 0xFF);
  fis->countl = sector_count & 0xFF;
  fis->counth = (sector_count >> 8) & 0xFF;

  send_cmd(device, cmdslot);  
}


static void device_init(sata_dev_t* device) {
  int cmdslot = find_cmdslot(device);

  if (cmdslot == -1) {
    PRINTK_SERIAL("[%s]: Cannot find free cmdslot right now..\n", MODULE_NAME);
    return;
  }

  device->port->cmd &= ~(1 << 0 | 1 << 8);
  while (device->port->cmd & (1 << 14) != 0 && device->port->cmd & (1 << 15) != 0);

  uint64_t cmdlist_buf = PAGE_ALIGN_UP(kmalloc(0x2000));
  uint64_t fb = PAGE_ALIGN_UP(kmalloc(0x1000));

  device->cmdlist_virt = cmdlist_buf;

  uint64_t fb_phys = VMM_PHYS(fb);

  device->port->clb = (uint32_t)VMM_PHYS(cmdlist_buf);
  device->port->clbu = VMM_PHYS(cmdlist_buf) >> 32;

  device->port->fb = (uint32_t)ALIGN_DOWN(fb_phys, 256);
  device->port->fbu = (uint64_t)(ALIGN_DOWN(fb_phys, 256)) >> 32;

  volatile HBA_CMD_HEADER* cmdheader = (HBA_CMD_HEADER*)cmdlist_buf;
  
  for (uint8_t i = 0; i < 32; ++i) {
    device->ctba_virts[i] = PAGE_ALIGN_UP(kmalloc(0x1000));
    cmdheader[i].ctba = (uint32_t)VMM_PHYS(device->ctba_virts[i]);
    cmdheader[i].ctbau = VMM_PHYS(device->ctba_virts[i]) >> 32;
    cmdheader[i].prdtl = 8;
  }

  while (device->port->cmd & (1 << 15) != 0);
  device->port->cmd |= (1 << 0 | 1 << 8);
}


static void find_ports(void) {
  if (sata_generic.ifaces != NULL) {
    return;
  }

  uint32_t port_count = abar->cap & 0x1F;
  n_slots = (abar->cap >> 8) & 0x1F;
  sata_generic.ifaces = kmalloc(sizeof(sata_dev_t) * port_count);
  sata_generic.iface_count = port_count;

  PRINTK_SERIAL("[%s]: HBA has %d ports and %d command slots.\n", MODULE_NAME, port_count, n_slots);
  sata_dev_t* devices = (sata_dev_t*)sata_generic.ifaces;

  for (uint32_t i = 0; i < port_count; ++i) {
    if (abar->pi & (1 << i) != 0) {
      HBA_PORT* port = (HBA_PORT*)&abar->ports[i];
      
      switch (check_type(port)) {
        case AHCI_DEV_SATA:
          devices[drive_count].port = port;
          devices[drive_count].magic = SATA_DEV_MAGIC;
          PRINTK_SERIAL("[%s]: SATA drive found @HBA_PORT_%d\n", MODULE_NAME, i);
          device_init(&devices[drive_count]);
          ++drive_count;
          break;
      }
    }
  }
}


static uint8_t take_ownership(void) {
  if (!(abar->cap & (1 << 0))) {
    PRINTK_SERIAL(PRINTK_RED "[%s] BIOS handoff not supported!\n", MODULE_NAME);
    return 1;
  }

  abar->bohc |= (1 << 1);
  
  while (abar->bohc & (1 << 0) == 0);

  for (uint32_t i = 0; i < 5; ++i) CLI_SLEEP;

  if (abar->bohc & (1 << 4)) {
    for (uint32_t i = 0; i < 10; ++i) CLI_SLEEP;
  }

  if (abar->bohc & (1 << 4) != 0 || abar->bohc & (1 << 0) != 0 || abar->bohc & (1 << 1) == 0) {
    PRINTK_SERIAL(PRINTK_RED "[%s]: BIOS handoff failure!\n", MODULE_NAME);
    return 1;
  }


  PRINTK_SERIAL("[%s]: BIOS handoff success!\n", MODULE_NAME);
  return 0;
}


void ahci_init(void)  {
  dev = pci_find_any(SATA_CONTROLLER_PCI_CLASS, SATA_CONTROLLER_PCI_SUBCLSS, -1);

  if (!(dev.valid)) {
    PRINTK_SERIAL("[%s]: No SATA controller attached!\n", MODULE_NAME);
    return;
  }

  printk("[%s]: SATA controller found on PCI bus %d, slot %d\n", MODULE_NAME, dev.bus, dev.slot);

  // Put some PCI information in the pci_dev struct.
  pci_dev.vendor_id = pci_read_vendorid(dev);
  pci_dev.device_id = pci_read_deviceid(dev);
  
  // Enable bus mastering.
  enable_bus_mastering(dev);
  printk("[%s]: Bus mastering enabled for SATA controller.\n", MODULE_NAME);

  // Get ABAR.
  abar = (HBA_MEM*)(uint64_t)dev.bar5;
  
  // Take control over the HBA.
  take_ownership();
  
  // Enable AHCI.
  abar->ghc |= (1 << 31);

  // Clear interrupt enabled bit.
  abar->ghc &= ~(1 << 1);

  printk("[%s]: HBA is in %s mode\n", MODULE_NAME, abar->ghc & GHC_AHCI_ENABLE ? "AHCI" : "IDE emulation");

  // Locate ports on the HBA.
  find_ports();

  // Init driver.
  driver_init(&sata_generic);

  if (drive_count == 0) {
    printk("[%s]: !!No drives attached!!\n", MODULE_NAME);
    return;
  } 
}
