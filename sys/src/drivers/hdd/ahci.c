#include <drivers/hdd/ahci.h>
#include <arch/bus/pci.h>
#include <lib/module.h>
#include <lib/log.h>
#include <lib/string.h>
#include <lib/math.h>
#include <lib/assert.h>
#include <mm/heap.h>
#include <mm/vmm.h>

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

static pci_device_t dev;
static HBA_MEM* abar = NULL;
sata_dev_t used_sata_dev;


static int find_cmdslot(HBA_PORT* p) {
  uint32_t slots = (p->sact | p->ci);
  uint32_t n_slots = (abar->cap >> 8) & 0xF0;

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


static void stop_cmd(HBA_PORT* port) {
  // Clear ST to remove the ability
  // of the HBA processing commands.
  port->cmd &= ~(HBA_PxCMD_ST);

  // Prevent received FISes from being
  // accepted by HBA.
  port->cmd &= ~(HBA_PxCMD_FRE);

  // Wait until the FIS Receive DMA engine 
  // and the command list DMA engine is no longer
  // running for this port.
  while (port->cmd & (HBA_PxCMD_FR | HBA_PxCMD_CR));
}


static void start_cmd(HBA_PORT* port) {
  // Wait until command list DMA engine is no longer
  // running for this port.
  while (port->cmd & (HBA_PxCMD_CR));

  // Allow received FISes to be accepted 
  // by the HBA and allow the HBA
  // to process commands.
  port->cmd |= HBA_PxCMD_FRE;
  port->cmd |= HBA_PxCMD_ST;
}

static void sata_read_at(HBA_PORT* port, uint64_t lba, uint32_t sector_count, uint16_t* buf) {
  int cmdslot = find_cmdslot(port);
  port->is = 0xFFFF;

  if (cmdslot == -1) {
    printk(PRINTK_RED "[%s]: No commandslots free!\n", MODULE_NAME);
    return;
  }
  
  HBA_CMD_HEADER* cmdheader = (HBA_CMD_HEADER*)used_sata_dev.clb_virtual;
  
  // Set cmd fis length and write bit.
  cmdheader[cmdslot].cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t);
  cmdheader[cmdslot].w = 0;
  cmdheader[cmdslot].c = 1;
  cmdheader[cmdslot].p = 1;
  
  // Get the command table.
  HBA_CMD_TBL* cmdtbl = (HBA_CMD_TBL*)used_sata_dev.ctba_virtual[cmdslot];
  kmemzero(cmdtbl, sizeof(HBA_CMD_TBL) + (cmdheader[cmdslot].prdtl-1)*sizeof(HBA_PRDT_ENTRY));

  // Set data base low/high.
  cmdtbl->prdt_entry[0].dba = VMM_PHYS(PAGE_ALIGN_DOWN(buf)) & 0xFFFFFFFF;
  cmdtbl->prdt_entry[0].dbau = VMM_PHYS(PAGE_ALIGN_DOWN(buf)) >> 32;


  ASSERT(cmdtbl->prdt_entry[0].dba != 0, "Fetching physical address failed.\n");

  // Set how many bytes we want to read.
  cmdtbl->prdt_entry[0].dbc = 512*sector_count-1;
  cmdtbl->prdt_entry[0].i = 0;

  // Setup FIS.
  FIS_REG_H2D* cmdfis = (FIS_REG_H2D*)(cmdtbl->cfis);
  cmdfis->fis_type = 0x27;      // Host to device.
  cmdfis->c = 1;
  cmdfis->command = 0x25;       // Read DMA extended.
  
  // Setup FIS LBA stuff.
  cmdfis->lba0 = lba & 0xFF;
  cmdfis->lba1 = (lba >> 8) & 0xFF;
  cmdfis->lba2 = (lba >> 16) & 0xFF;
  cmdfis->device = 64;      // LBA mode.
  cmdfis->lba3 = (lba >> 24) & 0xFF;
  cmdfis->lba4 = (lba >> 32) & 0xFF;
  cmdfis->lba5 = (lba >> 40) & 0xFF;
  
  // Setup sector count stuff.
  cmdfis->countl = sector_count & 0xFF;
  cmdfis->counth = sector_count >> 8;

  // Wait while BSY(bit 7) and DRQ(bit 3) aren't set.
  while (port->tfd & ((1 << 7) | (1 << 3)));
  
  // Issue the command.
  port->ci = 1 << cmdslot;

  while (1) {
    uint64_t is = port->is;
    // NOTE: Infinite loop on hardware is here.
    if (is & HBA_ALL_ERRORS) {
      printk(PRINTK_RED "[%s]: Disk read failed (error; port->is: %x)\n", MODULE_NAME, is);
      return;
    }

    if (!(port->ci & (1 << cmdslot))) {
      break;
    }
  }

  if (port->is & HBA_ALL_ERRORS) {
    printk(PRINTK_RED "[%s]: Disk read failed (error; port->is: %x)\n", MODULE_NAME);
    return;
  } 
}


static void port_init(HBA_PORT* port) {
  // Stop the command engine.
  stop_cmd(port);

  uintptr_t cmdlist_buf = ALIGN_UP((uintptr_t)kmalloc(0x800), 0x400);
  uintptr_t fis_buffer = ALIGN_UP((uintptr_t)kmalloc(0x200), 0x100);
  used_sata_dev.fb_virtual = fis_buffer;
  used_sata_dev.clb_virtual = cmdlist_buf;

  port->clb = VMM_PHYS(PAGE_ALIGN_DOWN(cmdlist_buf)) & 0xFFFFFFFF;
  port->clbu = VMM_PHYS(PAGE_ALIGN_DOWN(cmdlist_buf)) >> 32;

  ASSERT(port->clb != 0, "Fetching physical address failed.\n");

  port->fb = VMM_PHYS(PAGE_ALIGN_DOWN(fis_buffer)) & 0xFFFFFFFF;
  port->fbu = VMM_PHYS(PAGE_ALIGN_DOWN(fis_buffer)) >> 32;

  ASSERT(port->fb != 0, "Fetching physical address failed.\n");

  // Assign a command header to CLB vaddr.
  HBA_CMD_HEADER* cmdheader = (void*)cmdlist_buf;

  for (uint8_t i = 0; i < 32; ++i) {
    uintptr_t buf = ALIGN_UP((uintptr_t)kmalloc(512), 128);
    kmemzero((void*)buf, 512); 
    used_sata_dev.ctba_virtual[i] = buf;
    cmdheader[i].ctba = VMM_PHYS(PAGE_ALIGN_DOWN(buf)) & 0xFFFFFFFF;
    cmdheader[i].ctbau = VMM_PHYS(PAGE_ALIGN_DOWN(buf)) >> 32;
    cmdheader[i].prdtl = 8;
    ASSERT(cmdheader[i].ctba != 0, "Fetching physical address failed.\n");
  }
  
  // Start the command engine.
  start_cmd(port);
}


static void find_ports(void) {
  static uint8_t sata_dev_init = 0;
  uint32_t pi = abar->pi;

  for (uint8_t i = 0; i < 32; ++i, pi >>= 1) {
    if (pi & 1) {
      int dt = check_type(&abar->ports[i]);

      switch (dt) {
        case AHCI_DEV_SATA:
          if (!(sata_dev_init)) {
            used_sata_dev.port = &abar->ports[i];
            port_init(&abar->ports[i]);
            sata_dev_init = 1;
          }
          printk("[%s]: SATA drive found @HBA_PORT_%d\n", MODULE_NAME, i);
          break;
        case AHCI_DEV_SEMB:
          PRINTK_SERIAL("[%s]: Enclosure management bridge found @HBA_PORT_%d (ignoring)\n", MODULE_NAME, i);
          break;
        case AHCI_DEV_SATAPI:
          printk("[%s]: SATAPI drive found @HBA_PORT_%d (ignoring)\n", MODULE_NAME, i);
          break;
        case AHCI_DEV_PM:
          PRINTK_SERIAL("[%s]: Port multiplier found @HBA_PORT_%d (ignoring)\n", MODULE_NAME, i);
          break;
      }
    }
  }
}


void ahci_init(void)  {
  dev = pci_find_any(SATA_CONTROLLER_PCI_CLASS, SATA_CONTROLLER_PCI_SUBCLSS, -1);

  if (!(dev.valid)) {
    PRINTK_SERIAL("[%s]: No SATA controller attached!\n", MODULE_NAME);
    return;
  }

  printk("[%s]: SATA controller found on PCI bus %d, slot %d\n", MODULE_NAME, dev.bus, dev.slot);
  enable_bus_mastering(dev);
  printk("[%s]: Bus mastering enabled for SATA controller.\n", MODULE_NAME);

  abar = (HBA_MEM*)(uint64_t)dev.bar5;
  find_ports();
  
  printk("[%s]: HBA is in %s mode\n", MODULE_NAME, abar->ghc & GHC_AHCI_ENABLE ? "AHCI" : "IDE emulation");
  
  uint16_t* buf = (uint16_t*)(ALIGN_UP((uint64_t)kmalloc(1000), PAGE_SIZE));
  sata_read_at(used_sata_dev.port, 1, 1, buf);
}
