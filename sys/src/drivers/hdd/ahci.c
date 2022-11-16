#include <drivers/hdd/ahci.h>
#include <arch/bus/pci.h>
#include <lib/module.h>
#include <lib/log.h>
#include <lib/string.h>
#include <lib/math.h>
#include <mm/heap.h>
#include <mm/vmm.h>

#define AHCI_DEBUG 1

MODULE("ahci");

#define SATA_CONTROLLER_PCI_CLASS 0x01
#define SATA_CONTROLLER_PCI_SUBCLSS 0x06

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

#define HBA_PxCMD_ST    (1 << 0)
#define HBA_PxCMD_FRE   (1 << 4)
#define HBA_PxCMD_FR    (1 << 14)
#define HBA_PxCMD_CR    (1 << 15)

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
static HBA_PORT* sata = NULL;


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


static uint8_t read_disk_at(HBA_PORT* port, uint64_t lba, uint32_t sector_count, uint16_t* buf) {
  if (check_type(port) != AHCI_DEV_SATA)
    return 1;

  if (AHCI_DEBUG) {
    PRINTK_SERIAL("[%s] Reading SATA drive.. (LBA=%d, sector_count=%d)\n", MODULE_NAME, lba, sector_count);
  }

  uint64_t buf_phys = (uint64_t)buf - VMM_HIGHER_HALF;
  port->is = (uint32_t)-1;
  
  int32_t spin = 1000000;    // Spin lock counter to prevent getting stuck.
  int cmdslot = find_cmdslot(port);

  if (cmdslot == -1) {
    return 1;
  }

  uint64_t port_clb = ((uint64_t)port->clbu << 32 | port->clb) + VMM_HIGHER_HALF;
  HBA_CMD_HEADER* cmdheader = (HBA_CMD_HEADER*)port_clb;
  cmdheader[cmdslot].cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t);
  cmdheader[cmdslot].w = 0;

  uint64_t ctba = ((uint64_t)cmdheader[cmdslot].ctbau << 32 | cmdheader[cmdslot].ctba) + VMM_HIGHER_HALF;
  HBA_CMD_TBL* cmdtable = (HBA_CMD_TBL*)ctba;


  cmdtable->prdt_entry[0].dba = buf_phys & 0xFFFFFFFF;
  cmdtable->prdt_entry[0].dbau = buf_phys >> 32;
  cmdtable->prdt_entry[0].dbc = 512*sector_count-1;
  cmdtable->prdt_entry[0].i = 1;

  FIS_REG_H2D* cmdfis = (FIS_REG_H2D*)cmdtable->cfis;
  cmdfis->fis_type = 0x27;  // Host to device.
  cmdfis->c = 1;            // Command.
  cmdfis->command = 0x25;   // Read DMA extended.
  cmdfis->lba0 = lba & 0xFF;
  cmdfis->lba1 = (lba >> 8) & 0xFF;
  cmdfis->lba2 = (lba >> 16) & 0xFF;
  cmdfis->device = 64;      // LBA mode.
  cmdfis->lba3 = (lba >> 24) & 0xFF;
  cmdfis->lba4 = (lba >> 32) & 0xFF;
  cmdfis->lba5 = (lba >> 40) & 0xFF;
  cmdfis->countl = (sector_count & 0xFF);
  cmdfis->counth = (sector_count >> 8) & 0xFF;

  while (port->tfd & (1 << 7) && port->tfd & (1 << 3)) {
    --spin;

    if (!(spin)) {
      printk("[%s]: HBA port hung.\n", MODULE_NAME);
      return 1;
    }
  }

  port->ci = 1 << cmdslot;      // Issue command.
  
  while (1) {
    if (port->is & (1 << 30)) {
      printk("[%s]: Disk read error!\n", MODULE_NAME);
      return 1;
    }

    if (!(port->ci & (1 << cmdslot))) {
      break;
    }
  }

  if (port->is & (1 << 30)) {
      printk("[%s]: Disk read error!\n", MODULE_NAME);
      return 1;
  }

  return 0;
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


static void port_init(HBA_PORT* port) {
  // Stop the command engine.
  stop_cmd(port);
  
  // Setup the command list base.
  void* cmdlist_base = kmalloc(0x1000 + 0x3E8);
  kmemzero(cmdlist_base, 0x1000 + 0x3E8);

  uint64_t cmdlist_phys_base = (uint64_t)cmdlist_base - VMM_HIGHER_HALF;
  cmdlist_phys_base = ALIGN_UP(cmdlist_phys_base, 0x3E8);

  port->clb = cmdlist_phys_base & 0xFFFFFFFF;
  port->clbu = cmdlist_phys_base >> 32;

  // Set FB and FBU.
  void* fb_virt = kmalloc(0x1000 + 0x100);
  kmemzero(fb_virt, 0x1000 + 0x100);
  
  uint64_t fb_phys = (uint64_t)fb_virt - VMM_HIGHER_HALF;
  fb_phys = ALIGN_UP(fb_phys, 0x100);

  port->fb = fb_phys & 0xFFFFFFFF;
  port->fbu = fb_phys >> 32;

  // Setup the command header.
  HBA_CMD_HEADER* cmdheader = (HBA_CMD_HEADER*)cmdlist_base;

  for (int i = 0; i < 32; ++i) {
    cmdheader[i].prdtl = 8;         // 8 prdtl entries per command table.
                                    // 256-bytes per command table.
    
    void* ctba = kmalloc(0x2050);
    kmemzero(ctba, 256*32);

    uint64_t phys = (uint64_t)ctba - VMM_HIGHER_HALF;
    cmdheader[i].ctba = phys & 0xFFFFFFFF;
    cmdheader[i].ctbau = phys >> 32;
  }

  // Start the command engine.
  start_cmd(port);
}


static void find_ports(void) {
  uint32_t pi = abar->pi;

  for (uint8_t i = 0; i < 32; ++i, pi >>= 1) {
    if (pi & 1) {
      int dt = check_type(&abar->ports[i]);

      switch (dt) {
        case AHCI_DEV_SATA:
          if (sata == NULL) {
            sata = &abar->ports[i];
            port_init(&abar->ports[i]);
          }
          PRINTK_SERIAL("[%s]: SATA drive found @HBA_PORT_%d\n", MODULE_NAME, i);
          break;
        case AHCI_DEV_SEMB:
          PRINTK_SERIAL("[%s]: Enclosure management bridge found @HBA_PORT_%d (ignoring)\n", MODULE_NAME, i);
          break;
        case AHCI_DEV_SATAPI:
          PRINTK_SERIAL("[%s]: SATAPI drive found @HBA_PORT_%d (ignoring)\n", MODULE_NAME, i);
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

  PRINTK_SERIAL("[%s]: SATA controller found on PCI bus %d, slot %d\n", MODULE_NAME, dev.bus, dev.slot);
  enable_bus_mastering(dev);
  PRINTK_SERIAL("[%s]: Bus mastering enabled for SATA controller.\n", MODULE_NAME);

  abar = (HBA_MEM*)(uint64_t)dev.bar5;
  find_ports();
  
  uint16_t* buf = kmalloc(1000);
  read_disk_at(sata, 0, 1, buf);
  printk("%x->%x->%x\n", buf[0], buf[1], buf[2]);
}
