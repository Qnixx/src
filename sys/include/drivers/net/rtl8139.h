#ifndef RTL8139_H_
#define RTL8139_H_

#include <lib/types.h>
#include <net/ethernet.h>

/*
 *  RealTek 8139 Driver.
 */

// Packets
#define PACKET_SIZE_MAX 0x600
#define PACKET_SIZE_MIN 0x16

// Buffers
#define RX_BUFFER_SIZE 32768
#define TX_BUFFER_SIZE PACKET_SIZE_MAX
#define TX_BUFFER_COUNT 4

// Registers
#define REG_MAC       0x00
#define REG_MAR0      0x08
#define REG_MAR4      0x12
#define REG_TXSTATUS0 0x10
#define REG_TXADDR0   0x20
#define REG_RXBUF     0x30
#define REG_COMMAND   0x37
#define REG_CAPR      0x38
#define REG_IMR       0x3C
#define REG_ISR       0x3E
#define REG_TXCFG     0x40
#define REG_RXCFG     0x44
#define REG_MPC       0x4C
#define REG_CFG9346   0x50
#define REG_CONFIG1   0x52
#define REG_MSR       0x58
#define REG_BMCR      0x62
#define REG_ANLPAR    0x68

// Commands
#define CMD_RX_EMPTY  0x01
#define CMD_TX_ENABLE 0x04
#define CMD_RX_ENABLE 0x08
#define CMD_RESET     0x10

// Register config
#define CFG9346_NONE  0x00
#define CFG9346_EEM0  0x40
#define CFG9346_EEM1  0x80

// Basic mode control register
#define BMCR_SPEED          0x2000
#define BMCR_AUTO_NEGOTIATE 0x1000
#define BMCR_DUPLEX         0x0100

#define MSR_LINKB 0x02
#define MSR_SPEED_10 0x08
#define MSR_RX_FLOW_CONTROL_ENABLE 0x40

// Recieve configuration
#define RXCFG_AAP               0x01
#define RXCFG_APM               0x02
#define RXCFG_AM                0x04
#define RXCFG_AB                0x08
#define RXCFG_AR                0x10
#define RXCFG_WRAP_INHIBIT      0x80
#define RXCFG_MAX_DMA_16B       0x000
#define RXCFG_MAX_DMA_32B       0x100
#define RXCFG_MAX_DMA_64B       0x200
#define RXCFG_MAX_DMA_128B      0x300
#define RXCFG_MAX_DMA_256B      0x400
#define RXCFG_MAX_DMA_512B      0x500
#define RXCFG_MAX_DMA_1K        0x600
#define RXCFG_MAX_DMA_UNLIMITED 0x0700
#define RXCFG_RBLN_8K           0x0000
#define RXCFG_RBLN_16K          0x0800
#define RXCFG_RBLN_32K          0x1000
#define RXCFG_RBLN_64K          0x1800
#define RXCFG_FTH_NONE          0xE000

// Transmit configuration
#define TXCFG_TXRR_ZERO         0x00
#define TXCFG_MAX_DMA_16B       0x000
#define TXCFG_MAX_DMA_32B       0x100
#define TXCFG_MAX_DMA_64B       0x200
#define TXCFG_MAX_DMA_128B      0x300
#define TXCFG_MAX_DMA_256B      0x400
#define TXCFG_MAX_DMA_512B      0x500
#define TXCFG_MAX_DMA_1K        0x600
#define TXCFG_MAX_DMA_2K        0x700
#define TXCFG_IFG11             0x3000000

// Interrupts
#define INT_RXOK               0x01
#define INT_RXERR              0x02
#define INT_TXOK               0x04
#define INT_TXERR              0x08
#define INT_RX_BUFFER_OVERFLOW 0x10
#define INT_LINK_CHANGE        0x20
#define INT_RX_FIFO_OVERFLOW   0x40
#define INT_LENGTH_CHANGE      0x2000
#define INT_SYSTEM_ERROR       0x8000

// Transfer status
#define TX_STATUS_OWN           0x2000
#define TX_STATUS_THRESHOLD_MAX 0x3F0000

// Recieve status
#define RX_MULTICAST             0x8000
#define RX_PHYSICAL_MATCH        0x4000
#define RX_BROADCAST             0x2000
#define RX_INVALID_SYMBOL_ERROR  0x0020
#define RX_RUNT                  0x0010
#define RX_LONG                  0x0008
#define RX_CRC_ERROR             0x0004
#define RX_FRAME_ALIGNMENT_ERROR 0x0002
#define RX_OK                    0x0001

extern mac_address_t rtl8139_mac_addr;


void rtl8139_init(void);
void rtl8139_send_packet(void* data, size_t size);
void* rtl8139_read_packet(void);
uint8_t rtl8139_got_packet(void);

#endif
