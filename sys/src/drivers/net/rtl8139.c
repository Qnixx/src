#include <drivers/net/rtl8139.h>
#include <arch/bus/pci.h>
#include <arch/x86/io.h>
#include <arch/x86/apic/lapic.h>
#include <arch/x64/idt.h>
#include <lib/module.h>
#include <lib/log.h>
#include <mm/vmm.h>
#include <mm/heap.h>
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
static uint8_t txbufs[TX_BUFFER_COUNT];
static size_t next_txbuf = 0;
static pci_device_t dev;
static uint8_t mac_addr[6];


static inline uint8_t link_up(void) {
  return ((inb(iobase + REG_MSR) & MSR_LINKB) == 0);
}


static inline uint8_t get_speed(void) {
  uint16_t msr = inw(iobase + REG_MSR);
  return msr & MSR_SPEED_10 ? 10 : 100;
}

static void update_mac_addr(void) {
  for (unsigned int i = 0; i < 6; ++i) {
    mac_addr[i] = inb(iobase + REG_MAC + i);
  }
}

/*
static void reset_card(void) {
  PRINTK_SERIAL("[%s]: Software reset requested.\n", MODULE_NAME);
  next_txbuf = 0;

  // Reset the device.
  outb(iobase + REG_COMMAND, CMD_RESET);
  while (inb(iobase + REG_COMMAND) & CMD_RESET);
  PRINTK_SERIAL("[%s]: Card has been reset, re-configuring..\n", MODULE_NAME);
  
  // Unlock the config registers.
  outb(iobase + REG_CFG9346, CFG9346_EEM0 | CFG9346_EEM1);
  
  // Enable RX/TX.
  outb(iobase + REG_COMMAND, CMD_RX_ENABLE | CMD_TX_ENABLE);

  // Ensure the card isn't in sleep mode.
  outb(iobase + REG_CONFIG1, 0);

  PRINTK_SERIAL("[%s]: Configuration registers unlocked, RX/TX enabled, woke up card.\n", MODULE_NAME);

  // Setup the RX buffer.
  uint64_t rxbuf_vaddr = (uint64_t)&rxbuf;
  outl(iobase + REG_RXBUF, rxbuf_vaddr - VMM_HIGHER_HALF);
  
  // Reset missed packet count.
  outb(iobase + REG_MPC, 0);

  // Basic mode control configuration (100mbit full duplex auto negoiation mode).
  outl(iobase + REG_BMCR, BMCR_SPEED | BMCR_AUTO_NEGOTIATE | BMCR_DUPLEX);

  // Enable control flow.
  outb(iobase + REG_MSR, MSR_RX_FLOW_CONTROL_ENABLE);

  // Set RX mode: accept rtl8139 mac match, multicast and broadcasted packets.
  // We will also use DMA transfer size and no FIFO threshold.
  outl(iobase + REG_RXCFG, RXCFG_APM | RXCFG_AM | RXCFG_AB | RXCFG_WRAP_INHIBIT | RXCFG_MAX_DMA_UNLIMITED | RXCFG_RBLN_32K | RXCFG_FTH_NONE);

  PRINTK_SERIAL("[%s]: RX MODE => Accept mac match, multicast and broadcasted packets, use DMA transfer size and disallow FIFO threshold.\n", MODULE_NAME);

  // Set TX mode: default retry count, max DMA burst size and interface gap time.
  outl(iobase + REG_TXCFG, TXCFG_TXRR_ZERO | TXCFG_MAX_DMA_1K | TXCFG_IFG11);

  PRINTK_SERIAL("[%s]: TX MODE => Default retry count, max DMA burst size and interface gap time.\n", MODULE_NAME);

  // Setup TX buffers.
  for (int i = 0; i < TX_BUFFER_COUNT; ++i) {
    uint64_t tx_buf_vaddr = (uint64_t)txbufs[i];
    outl(iobase + REG_TXADDR0 + (i * 4), tx_buf_vaddr - VMM_HIGHER_HALF);
  }

  // Lock configuration registers.
  outb(iobase + REG_CFG9346, CFG9346_NONE);

  // Re-enable RX/TX because the card could have
  // done a funny and disabled them.
  outw(iobase + REG_IMR, INT_RXOK | INT_RXERR | INT_TXOK | INT_TXERR | INT_RX_BUFFER_OVERFLOW | INT_LINK_CHANGE | INT_RX_FIFO_OVERFLOW | INT_LENGTH_CHANGE | INT_SYSTEM_ERROR);
  outw(iobase + REG_ISR, 0xFFFF);

  // enable_bus_mastering(dev);
  // printk("[%s]: Bus mastering enabled for the NIC.\n", MODULE_NAME);
}
*/

__attribute__((interrupt)) static void isr(void* stackframe) {
  for(;;) {
    uint16_t status = inw(iobase + REG_ISR);
    outw(iobase + REG_ISR, status);

    if  ((status & (INT_RXOK | INT_RXERR | INT_TXOK | INT_TXERR | INT_RX_BUFFER_OVERFLOW | INT_LINK_CHANGE | INT_RX_FIFO_OVERFLOW | INT_LENGTH_CHANGE | INT_SYSTEM_ERROR)) == 0) break;

    if (status & INT_RXOK) {
      PRINTK_SERIAL("[%s]: RX ready\n", MODULE_NAME);
    } 

    if (status & INT_TXOK) {
      PRINTK_SERIAL("[%s]: TX complete\n", MODULE_NAME);
    }

    if (status & INT_RX_BUFFER_OVERFLOW) {
      PRINTK_SERIAL("[%s]: RX buffer overflow\n", MODULE_NAME);
    }

    if (status & INT_LINK_CHANGE) { 
      PRINTK_SERIAL("[%s]: Link status changed, STATE=%s\n", MODULE_NAME, link_up() ? "UP" : "DOWN");
    } 
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
  if (size < 60) size = 60;
  
  uint64_t phys_addr = txbufs[hwbuf];
  kmemzero((void*)((txbufs[hwbuf] + VMM_HIGHER_HALF) + size), TX_BUFFER_SIZE - size);
  kmemcpy((void*)(phys_addr + VMM_HIGHER_HALF), data, size);
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

  outb(iobase + REG_COMMAND, CMD_RESET);
  while (inb(iobase + REG_COMMAND) & CMD_RESET);

  /*
   *  Setup RX and TX buffers.
   *  (needs to be physical address, hence the subtraction by VMM_HIGHER_HALF).
   *
   */
  outl(iobase + REG_RXBUF, (uintptr_t)(&rxbuf) - VMM_HIGHER_HALF);

  for (unsigned int i = 0; i < TX_BUFFER_COUNT; ++i) {
    txbufs[i] = (uint64_t)kmalloc(TX_BUFFER_SIZE) - VMM_HIGHER_HALF;
  }
  
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
   *  Enable Receive and Transmit.
   *
   */

  outb(iobase + REG_COMMAND, 0x0C);
  PRINTK_SERIAL("[%s]: Receive enable and Transmit enable set, card now can propagate packets.\n", MODULE_NAME);
  PRINTK_SERIAL("[%s]: Verifying link status..\n", MODULE_NAME);

  if (link_up()) {
    PRINTK_SERIAL("[%s]: Link up @%dmbps!\n", MODULE_NAME, get_speed());
  } else {
    PRINTK_SERIAL("[%s]: Link down.\n", MODULE_NAME);
    return;
  }

  update_mac_addr();
  printk("[%s]: MAC address: %X:%X:%X:%X:%X:%X\n", MODULE_NAME, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  register_irq(dev.irq_line, isr, 0); 
}
