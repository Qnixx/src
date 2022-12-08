#include <arch/x86/apic/lapic.h>
#include <arch/x86_64/idt.h>
#include <drivers/timer/pit.h>
#include <intr/irq.h>
#include <lib/log.h>

// Local APIC Registers
#define LAPIC_ID                        0x0020  // Local APIC ID
#define LAPIC_VER                       0x0030  // Local APIC Version
#define LAPIC_TPR                       0x0080  // Task Priority
#define LAPIC_APR                       0x0090  // Arbitration Priority
#define LAPIC_PPR                       0x00a0  // Processor Priority
#define LAPIC_EOI                       0x00b0  // EOI
#define LAPIC_RRD                       0x00c0  // Remote Read
#define LAPIC_LDR                       0x00d0  // Logical Destination
#define LAPIC_DFR                       0x00e0  // Destination Format
#define LAPIC_SVR                       0x00f0  // Spurious Interrupt Vector
#define LAPIC_ISR                       0x0100  // In-Service (8 registers)
#define LAPIC_TMR                       0x0180  // Trigger Mode (8 registers)
#define LAPIC_IRR                       0x0200  // Interrupt Request (8 registers)
#define LAPIC_ESR                       0x0280  // Error Status
#define LAPIC_ICRLO                     0x0300  // Interrupt Command
#define LAPIC_ICRHI                     0x0310  // Interrupt Command [63:32]
#define LAPIC_TIMER                     0x0320  // LVT Timer
#define LAPIC_THERMAL                   0x0330  // LVT Thermal Sensor
#define LAPIC_PERF                      0x0340  // LVT Performance Counter
#define LAPIC_LINT0                     0x0350  // LVT LINT0
#define LAPIC_LINT1                     0x0360  // LVT LINT1
#define LAPIC_ERROR                     0x0370  // LVT Error
#define LAPIC_TICR                      0x0380  // Initial Count (for Timer)
#define LAPIC_TCCR                      0x0390  // Current Count (for Timer)
#define LAPIC_TDCR                      0x03e0  // Divide Configuration (for Timer)

// Delivery Mode
#define ICR_FIXED                       0x00000000
#define ICR_LOWEST                      0x00000100
#define ICR_SMI                         0x00000200
#define ICR_NMI                         0x00000400
#define ICR_INIT                        0x00000500
#define ICR_STARTUP                     0x00000600

// Destination Mode
#define ICR_PHYSICAL                    0x00000000
#define ICR_LOGICAL                     0x00000800

// Delivery Status
#define ICR_IDLE                        0x00000000
#define ICR_SEND_PENDING                0x00001000

// Level
#define ICR_DEASSERT                    0x00000000
#define ICR_ASSERT                      0x00004000

// Trigger Mode
#define ICR_EDGE                        0x00000000
#define ICR_LEVEL                       0x00008000

// Destination Shorthand
#define ICR_NO_SHORTHAND                0x00000000
#define ICR_SELF                        0x00040000
#define ICR_ALL_INCLUDING_SELF          0x00080000
#define ICR_ALL_EXCLUDING_SELF          0x000c0000

extern void* lapic_base;

static void write(uint16_t reg, uint32_t value) {
  *(volatile uint32_t*)(lapic_base + reg) = value;
}

static uint32_t read(uint16_t reg) {
  return *(volatile uint32_t*)(lapic_base + reg);
}


static void lapic_timer_stop(void) {
  write(LAPIC_TICR, 0);
  write(LAPIC_TIMER, 1 << 16);
}


static void init_lapic_timer(void) {
  lapic_timer_stop();
  register_irq(0, _irq0);

  // Setup PIT.
  pit_set_count(0xFFFF);

  // Set up the timer.
  write(LAPIC_TIMER, (1 << 16) | 0xFF);
  write(LAPIC_TDCR, 0);

  uint32_t init_tick = pit_get_count();
  uint32_t samples = 0xFFFFF;
  write(LAPIC_TICR, samples);
  
  // Wait until TCCR is zero.
  ASMV("sti");
  while (read(LAPIC_TCCR) != 0);
  
  uint32_t final_tick = pit_get_count();
  uint32_t total_ticks = init_tick - final_tick;

  printk("[lapic]: Local APIC timer frequency: @%dGHz\n", (((samples/total_ticks)*PIT_DIVIDEND))/1000000000);
}


void lapic_send_ipi(uint8_t apic_id, uint8_t vector) {
  while (read(LAPIC_VER) & ICR_SEND_PENDING);
  uint32_t control = vector | ICR_ASSERT;
  write(LAPIC_ICRHI, (uint32_t)apic_id << 24);
  write(LAPIC_ICRLO, control);
}


void lapic_send_init(uint8_t apic_id) {
  while (read(LAPIC_VER) & ICR_SEND_PENDING);
  write(LAPIC_ICRHI, apic_id << 24);
  write(LAPIC_ICRLO, ICR_INIT
                     | ICR_PHYSICAL
                     | ICR_ASSERT
                     | ICR_EDGE
                     | ICR_NO_SHORTHAND);

  while (read(LAPIC_ICRLO) & ICR_SEND_PENDING);
}


void lapic_send_startup(uint8_t apic_id, uint8_t vector) {
  while (read(LAPIC_VER) & ICR_SEND_PENDING);
  write(LAPIC_ICRHI, apic_id << 24);
  write(LAPIC_ICRLO, ICR_STARTUP
                     | ICR_PHYSICAL
                     | ICR_ASSERT
                     | ICR_EDGE
                     | ICR_NO_SHORTHAND);

  while (read(LAPIC_ICRLO) & ICR_SEND_PENDING);
}

uint32_t lapic_read_id(void) {
  return read(LAPIC_ID) >> 24;
}

void lapic_send_eoi(void) {
  write(LAPIC_EOI, 0);
}

void lapic_init(void) {
  // Use flat model.
  write(LAPIC_DFR, 0xFFFFFFFF);
  
  // All cores will use logical ID of 1.
  write(LAPIC_LDR, 0x01000000);

  /*
   *  Enable the local APIC for the current core.
   *
   */

  write(LAPIC_SVR, (1 << 8) | 0xFF);
  init_lapic_timer();
}
