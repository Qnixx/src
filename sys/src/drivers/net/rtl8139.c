#include <drivers/net/rtl8139.h>
#include <arch/bus/pci.h>
#include <arch/x86/io.h>
#include <arch/x86/apic/lapic.h>
#include <arch/x64/idt.h>
#include <lib/module.h>
#include <lib/log.h>
#include <mm/vmm.h>
#include <lib/string.h>

MODULE("rtl8139");


#define VENDOR_ID 0x10EC
#define DEVICE_ID 0x8139


/*
 *  RealTek 8139 Driver.
 *
 */


static uint32_t iobase = 0;
static uint8_t rxbuf[RX_BUFFER_SIZE];
// static uint8_t txbuf[TX_BUFFER_SIZE];
static uint8_t txbufs[TX_BUFFER_SIZE][TX_BUFFER_COUNT];
static size_t next_txbuf = 0;
static pci_device_t dev;


static inline uint8_t link_up(void) {
  return ((inb(iobase + REG_MSR) & MSR_LINKB) == 0);
}


static inline uint8_t get_speed(void) {
  uint16_t msr = inw(iobase + REG_MSR);
  return msr & MSR_SPEED_10 ? 10 : 100;
}

__attribute__((interrupt)) static void isr(void* stackframe) {
  for(;;) {
    uint16_t status = inw(iobase + REG_ISR);
    outw(iobase + REG_ISR, status);

    if  ((status & (INT_RXOK | INT_RXERR | INT_TXOK | INT_TXERR | INT_RX_BUFFER_OVERFLOW | INT_LINK_CHANGE | INT_RX_FIFO_OVERFLOW | INT_LENGTH_CHANGE | INT_SYSTEM_ERROR)) == 0) break;
  }

  lapic_send_eoi();
}


void rtl8139_send_packet(void* data, size_t size) { 
  if (iobase == 0)
    return;
  
  ssize_t hwbuf = -1;

  for (unsigned int i = 0; i < TX_BUFFER_COUNT; ++i) {
    size_t canidate = (next_txbuf + i) % 4;
    uint32_t status = inl(iobase + REG_TXSTATUS0 + (canidate * 4));

    if (status & TX_STATUS_OWN) {
      hwbuf = canidate;
      break;
    }
  }

  next_txbuf = (hwbuf + 1) % 4;
  kmemcpy(txbufs[hwbuf], data, size);
  kmemzero(txbufs[hwbuf], TX_BUFFER_SIZE - size);
  if (size < 60) size = 60;
  outl(iobase + REG_TXSTATUS0 + (hwbuf * 4), size);
}


void rtl8139_init(void) {
  PRINTK_SERIAL("[%s]: Checking the existance of a RTL8139 card..\n", MODULE_NAME);
  dev = pci_find(VENDOR_ID, DEVICE_ID);

  if (!(dev.valid)) {
    PRINTK_SERIAL("[%s]: No RTL8139 card attached!\n", MODULE_NAME);
    return;
  }

  printk("[%s]: RTL8139 card is attached on PCI bus %d, slot %d\n", MODULE_NAME, dev.bus, dev.slot);
  enable_bus_mastering(dev);
  printk("[%s]: Bus mastering enabled for the NIC.\n", MODULE_NAME);

  /*
   *  Fetch I/O base.
   */

  iobase = dev.bar0 & 0xFFFFFFFc;
  PRINTK_SERIAL("[%s]: Card has I/O base @%x\n", MODULE_NAME, iobase);

  /*
   *  Now we have to turn on the
   *  card.
   *
   *  This can be done by sending a null byte
   *  to CONFIG1.
   */

  outb(iobase + REG_CONFIG1, 0x00);
  printk("[%s]: Turned on RTL8139.\n", MODULE_NAME);

  /*
   *  We will now do a software reset
   *  to clear any garbage left in the
   *  registers and stuff.
   *
   *  This can be done by sending 0x10 to
   *  the command register.
   *
   *  The RST bit must be checked to 
   *  make sure that the chip has finished the 
   *  reset. If the RST bit is high (1), then the reset is still 
   *  in operation.
   *
   */

  outb(iobase + REG_COMMAND, 0x10);
  while (inb(iobase + REG_COMMAND) & CMD_RESET);

  /*
   *  Setup RX buffer 
   *  (needs to be physical address, hence the subtraction by VMM_HIGHER_HALF).
   *
   */
  outl(iobase + REG_RXBUF, (uintptr_t)(&rxbuf) - VMM_HIGHER_HALF);
  PRINTK_SERIAL("[%s]: RX and TX buffer set.\n", MODULE_NAME);

  /*
   *  Set IMR TOK (Transmit OK) and ROK (Receive OK) bits to high.
   *
   */

  outw(iobase + REG_IMR, (uintptr_t)0x0005);

  /*
   * Setup RXCFG.
   *
   */

  outl(iobase + REG_RXCFG, RXCFG_APM | RXCFG_AM | RXCFG_AB | RXCFG_WRAP_INHIBIT | RXCFG_MAX_DMA_UNLIMITED | RXCFG_RBLN_32K | RXCFG_FTH_NONE);

  /*
   *  Enable Receive and Transmitter.
   *
   */

  outb(iobase + REG_COMMAND, 0x0C);
  PRINTK_SERIAL("[%s]: Receive enable and Transmit enable set, card now can propagate packets.\n", MODULE_NAME);
  PRINTK_SERIAL("[%s]: Verifying link status..\n", MODULE_NAME);

  if (link_up()) {
    PRINTK_SERIAL("[%s]: Link up @%dmbps!\n", MODULE_NAME, get_speed());
  } else {
    PRINTK_SERIAL("[%s]: Link down.\n", MODULE_NAME);
  }

  register_irq(dev.irq_line, isr, 0); 
}
