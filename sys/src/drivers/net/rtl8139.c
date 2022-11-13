#include <drivers/net/rtl8139.h>
#include <net/ethernet.h>
#include <arch/bus/pci.h>
#include <arch/x86/io.h>
#include <arch/x86/apic/lapic.h>
#include <arch/x64/idt.h>
#include <lib/module.h>
#include <lib/log.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <lib/string.h>
#include <lib/math.h>

MODULE("rtl8139");


#define VENDOR_ID 0x10EC
#define DEVICE_ID 0x8139


/*
 *  RealTek 8139 Driver.
 *
 */


static uint32_t iobase = 0;
static uint8_t txbufs[TX_BUFFER_COUNT];
static size_t next_txbuf = 0;
static ssize_t rxbuf_offset = 0;
static uint8_t got_packet = 0;
static pci_device_t dev;
static uint8_t* rxbuf = NULL;         // Must be RX_BUFFER_SIZE of size.
static void* packet_buf = NULL;
mac_address_t rtl8139_mac_addr;


static inline uint8_t link_up(void) {
  return ((inb(iobase + REG_MSR) & MSR_LINKB) == 0);
}


static inline uint8_t get_speed(void) {
  uint16_t msr = inw(iobase + REG_MSR);
  return msr & MSR_SPEED_10 ? 10 : 100;
}

static void update_mac_addr(void) {
  for (unsigned int i = 0; i < 6; ++i) {
    rtl8139_mac_addr[i] = inb(iobase + REG_MAC + i);
  }
}


static void recieve(void) {
  uint8_t* packet = rxbuf + rxbuf_offset;
  uint16_t status = *(uint16_t*)(packet + 0);
  uint16_t length = *(uint16_t*)(packet + 2);

  if (!(status & RX_OK) || (status & (RX_INVALID_SYMBOL_ERROR | RX_CRC_ERROR | RX_FRAME_ALIGNMENT_ERROR)) || (length >= PACKET_SIZE_MAX) || (length < PACKET_SIZE_MIN)) {
    printk("[%s]: Got bad packet (status=%x, length=%x)\n", MODULE_NAME, status, length);
    return;
  }

  kmemcpy(packet_buf, (uint8_t*)(packet + 4), length - 4);
  rxbuf_offset = ((rxbuf_offset + length + 4 + 3) & ~3) % RX_BUFFER_SIZE;
  outw(iobase + REG_CAPR, rxbuf_offset - 0x10);
  rxbuf_offset %= RX_BUFFER_SIZE;

  PRINTK_SERIAL("[%s]: Recieved %d bytes of data.\n", MODULE_NAME, length);
}

__attribute__((interrupt)) static void isr(void* stackframe) {
  for(;;) {
    uint16_t status = inw(iobase + REG_ISR);
    outw(iobase + REG_ISR, status);

    if  ((status & (INT_RXOK | INT_RXERR | INT_TXOK | INT_TXERR | INT_RX_BUFFER_OVERFLOW | INT_LINK_CHANGE | INT_RX_FIFO_OVERFLOW | INT_LENGTH_CHANGE | INT_SYSTEM_ERROR)) == 0) break;

    if (status & INT_TXOK) {
      PRINTK_SERIAL("[%s]: TX complete.\n", MODULE_NAME);
    }

    if (status & INT_TXERR) {
      printk("[%s]: TX error.\n", MODULE_NAME);
    }

    if (status & INT_RXOK) {
      recieve();
      got_packet = 1;
    }

    if (status & INT_RXERR) {
      printk("[%s]: RX error.\n", MODULE_NAME);
    }

    if (status & INT_RX_BUFFER_OVERFLOW) {
      printk("[%s]: RX buffer overflow.\n", MODULE_NAME);
    }

    if (status & INT_LINK_CHANGE) { 
      printk("[%s]: Link status changed, STATE=%s\n", MODULE_NAME, link_up() ? "UP" : "DOWN");
    } 
  }

  outw(iobase + REG_ISR, 0x5);
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
  uint64_t virt_addr = phys_addr + VMM_HIGHER_HALF;
  kmemzero((void*)(ALIGN_DOWN(virt_addr, 0x1000)), TX_BUFFER_SIZE - size);
  kmemcpy((void*)(ALIGN_DOWN(virt_addr, 0x1000)), data, size);
  outl(iobase + REG_TXSTATUS0 + (hwbuf * 4), size);
}


void* rtl8139_read_packet(void) {
  return packet_buf;
}


uint8_t rtl8139_got_packet(void) {
  if (!(got_packet))
    return 0;

  uint8_t tmp = got_packet;
  got_packet = 0;
  return tmp;
}


void rtl8139_init(void) {
  PRINTK_SERIAL("[%s]: Checking the existance of a RTL8139 card..\n", MODULE_NAME);
  dev = pci_find(VENDOR_ID, DEVICE_ID);

  if (!(dev.valid)) {
    PRINTK_SERIAL("[%s]: No RTL8139 card attached!\n", MODULE_NAME);
    return;
  }

  printk("[%s]: RTL8139 card is attached on PCI bus %d, slot %d\n", MODULE_NAME, dev.bus, dev.slot);
  
  // Enable bus mastering.
  enable_bus_mastering(dev);
  printk("[%s]: Bus mastering enabled for the NIC.\n", MODULE_NAME);

  // Fetch the I/O base.
  iobase = dev.bar0 & 0xFFFFFFFC;
  PRINTK_SERIAL("[%s]: Card has I/O base @%x\n", MODULE_NAME, iobase);

  // Reset the card to ensure no garbage is in the registers
  // and stuff.
  outb(iobase + REG_COMMAND, CMD_RESET);
  while (inb(iobase + REG_COMMAND) & CMD_RESET);

  // Unlock config registers.
  outb(iobase + REG_CFG9346, CFG9346_EEM0 | CFG9346_EEM1);

  // Turn on multicast.
  outl(iobase + REG_MAR0, 0xFFFFFFFF);
  outl(iobase + REG_MAR4, 0xFFFFFFFF);

  PRINTK_SERIAL("[%s]: Multicast enabled.\n", MODULE_NAME);

  // Enable RX and TX.
  outb(iobase + REG_COMMAND, CMD_RX_ENABLE | CMD_TX_ENABLE);
  PRINTK_SERIAL("[%s]: RX and TX enabled.\n", MODULE_NAME);

  // Turn on the card.
  outb(iobase + REG_CONFIG1, 0x00);
  printk("[%s]: Turned on RTL8139.\n", MODULE_NAME);

  // Allocate memory for the RX buffer.
  rxbuf = kmalloc(RX_BUFFER_SIZE);
  uintptr_t rxbuf_phys = (uint64_t)rxbuf - VMM_HIGHER_HALF;

  // Setup RX buffer.
  // (needs to be physical address, hence the subtraction by VMM_HIGHER_HALF).
  outl(iobase + REG_RXBUF, rxbuf_phys);
  PRINTK_SERIAL("[%s]: RX buffer have been set up.\n", MODULE_NAME);
  
  // Allocate memory for packet buffer.
  packet_buf = kmalloc(PACKET_SIZE_MAX);

  // Reset missed packet count which basically
  // is the number of packets discarded due to RX FIFO overflow.
  outb(iobase + REG_MPC, 0);
  PRINTK_SERIAL("[%s]: MPC set to zero.\n", MODULE_NAME);

  // Basic mode control configuration, 100mbit full duplex auto negoiation mode
  outl(iobase + REG_BMCR, BMCR_SPEED  | BMCR_AUTO_NEGOTIATE | BMCR_DUPLEX);

  // Enable control flow.
  outb(iobase + REG_MSR, MSR_RX_FLOW_CONTROL_ENABLE);

  // Set RX mode: accept rtl8139 MAC match, multicast, and broadcasted packets
  // Also use max DMA transfer size and no FIFO threshold
  outl(iobase + REG_RXCFG, RXCFG_APM | RXCFG_AM | RXCFG_AB | RXCFG_WRAP_INHIBIT | RXCFG_MAX_DMA_UNLIMITED | RXCFG_RBLN_32K | RXCFG_FTH_NONE);
  PRINTK_SERIAL("[%s]: RX_MODE => accept rtl8139 MAC match, multicast and broadcasted packets.\n", MODULE_NAME);
  PRINTK_SERIAL("[%s]: RX_MODE_CONTINUED => Use max DMA transfer size and no FIFO threshold.\n", MODULE_NAME);

  // Set TX mode to use default retry count, max DMA burst size and interframe gap time.
  outl(iobase + REG_TXCFG, TXCFG_TXRR_ZERO | TXCFG_MAX_DMA_1K | TXCFG_IFG11);
  PRINTK_SERIAL("[%s]: TX_MODE => Use default retry count, max DMA burst size, interframe gap time.\n", MODULE_NAME);

  // Setup TX buffers.
  for (unsigned int i = 0; i < TX_BUFFER_COUNT; ++i) {
    txbufs[i] = (uint64_t)kmalloc(TX_BUFFER_SIZE) - VMM_HIGHER_HALF;
  }

  PRINTK_SERIAL("[%s]: TX buffers have been set up.\n", MODULE_NAME);

  // Re-lock configuration registers.
  outb(iobase + REG_CFG9346, CFG9346_NONE);
  PRINTK_SERIAL("[%s]: Locked configuration registers.\n", MODULE_NAME);

  // Re-enable RX/TX because the card
  // sometimes does a funny and disables them.
  outw(iobase + REG_IMR, INT_RXOK | INT_RXERR | INT_TXOK | INT_TXERR | INT_RX_BUFFER_OVERFLOW | INT_LINK_CHANGE | INT_RX_FIFO_OVERFLOW | INT_LENGTH_CHANGE | INT_SYSTEM_ERROR);
  outw(iobase + REG_ISR, 0xFFFF);
  
  if (link_up()) {
    PRINTK_SERIAL("[%s]: Link up @%dmbps!\n", MODULE_NAME, get_speed());
  } else {
    PRINTK_SERIAL("[%s]: Link down.\n", MODULE_NAME);
    return;
  }

  update_mac_addr();
  printk("[%s]: MAC address: %X:%X:%X:%X:%X:%X\n", MODULE_NAME, rtl8139_mac_addr[0], rtl8139_mac_addr[1], rtl8139_mac_addr[2], rtl8139_mac_addr[3], rtl8139_mac_addr[4], rtl8139_mac_addr[5]);
  register_irq(dev.irq_line, isr, 0); 
}
