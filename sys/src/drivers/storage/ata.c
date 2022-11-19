#include <drivers/storage/ata.h>
#include <arch/x86/io.h>
#include <lib/module.h>
#include <lib/log.h>

MODULE("ata");

#define MASTER_DRIVE 0xA0
#define DRIVE_SELECT 0x1F6
#define STATUS_COMMAND 0x1F7      // Read (status), write (command).
#define CTRL 0x3F6

#define STATUS_BSY (1 << 7)
#define STATUS_RDY (1 << 6)
#define STATUS_DRQ (1 << 3)
#define STATUS_DF  (1 << 5)
#define STATUS_ERR (1 << 0)

static inline void wait(void) {
  for (uint32_t i = 0; i < 7; ++i) {
    io_wait();
  }
}

static inline void reset(void) {
  outb(CTRL, inb(CTRL) | (1 << 2));
  wait();
  outb(CTRL, inb(CTRL) & ~(1 << 2));
}

static void wait_busy(void) {
  while (inb(STATUS_COMMAND) & STATUS_BSY);
}

static void wait_drq(void) {
  while (!(inb(STATUS_COMMAND) & STATUS_RDY));
}

void ata_write(uint16_t* in_buf, uint32_t lba, uint16_t sector_count) {
  wait_busy();
  outb(DRIVE_SELECT, 0xE0);
  outb(0x1F2, sector_count);
  outb(0x1F3, lba & 0xFF);
  outb(0x1F4, (lba >> 8) & 0xFF);
  outb(0x1F5, (lba >> 16) & 0xFF);
  outb(STATUS_COMMAND, 0x30);       // WRITE.

  for (uint16_t i = 0; i < sector_count; ++i) {
    wait_busy();
    wait_drq();

    for (uint16_t j = 0; j < 256; ++j) {
      outl(0x1F0, in_buf[j]);
    }
  }
}

void ata_init(void) {
  /*
   *  Check for the floating bus
   *  condition (i.e no drives on bus).
   *
   */
  if (inb(STATUS_COMMAND) == 0xFF) {
    PRINTK_SERIAL("[%s]: No ATA drives on bus.\n", MODULE_NAME);
    return;
  }

  printk("[%s]: ATA drive has been detected on bus.\n", MODULE_NAME);
}
