#ifndef AHCI_H_
#define AHCI_H_

#include <lib/types.h>


typedef volatile struct HBA_PORT {
  uint32_t clb;       // Command list base address (1K-byte aligned).
  uint32_t clbu;      // Command list base address (upper 32 bits).
  uint32_t fb;        // FIS base address, 256-byte aligned.
  uint32_t fbu;       // FIS base address upper 32 bits.
  uint32_t is;        // Interrupt status.
  uint32_t ie;        // Interrupt enable.
  uint32_t cmd;       // Command and status.
  uint32_t rsv0;      // Reserved.
  uint32_t tfd;       // Task file data.
  uint32_t sig;       // Signature.
  uint32_t ssts;      // SATA status.
  uint32_t sctl;      // SATA control.
  uint32_t serr;      // SATA error.
  uint32_t sact;      // SATA active.
  uint32_t ci;        // Command issue.
  uint32_t sntf;      // SATA notification.
  uint32_t fbs;       // FIS based switch control.
  uint32_t rsv1[11];  // Reserved.
  uint32_t vendor[4]; // Vendor specfic.
} HBA_PORT;

typedef volatile struct HBA_MEM {
  uint32_t cap;         // Host capability.
  uint32_t ghc;         // Global host control.
  uint32_t is;          // Interrupt status.
  uint32_t pi;          // Port implemented.
  uint32_t vs;          // Version.
  uint32_t ccc_ctl;     // Command completion coalescing control
  uint32_t ccc_ports;   // Command completion coalescing ports.
  uint32_t em_loc;      // Enclosure management location.
  uint32_t em_ctl;      // Enclosure management control.
  uint32_t cap2;        // Command completion coalescing ports.
  uint32_t bohc;        // BIOS/OS handoff control and status.
  
  uint8_t reserved[0xA0-0x2C];
  uint8_t vendor[0x100-0xA0];
  HBA_PORT ports[1];    // 1 ~ 32.
} HBA_MEM;


typedef struct HBA_CMD_HEADER {
  uint8_t cfl : 5;          // CMD FIS length in dwords.
  uint8_t a : 1;            // ATAPI.
  uint8_t w : 1;            // Write, 1: H2D, 0: D2H.
  uint8_t p : 1;            // Prefetchable.
  uint8_t r : 1;            // Reset.
  uint8_t b : 1;            // BIST.
  uint8_t c : 1;            // Clear busy upon R_OK.
  uint8_t rsv0 : 1;         // Reserved.
  uint8_t pmp : 4;          // Port multiplier port.

  uint16_t prdtl;           // Physical region descriptor table length in entries.
  volatile uint32_t prdbc;  // Physical region descriptor byte count transferred.
  uint32_t ctba;            // Command table descriptor base address.
  uint32_t ctbau;           // Command table descriptor base address upper 32 bits.
  uint32_t rsv1[4];         // Reserved.
} HBA_CMD_HEADER;


void ahci_init(void);


#endif
