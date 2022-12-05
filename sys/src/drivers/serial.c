#include <drivers/serial.h>
#include <arch/x86/io.h>
#include <lib/types.h>
#include <lib/string.h>
#include <fs/devfs.h>
#include <lib/asm.h>

#define COM1 0x3F8

static uint8_t faulty = 0;

static uint8_t is_transmit_empty(void) {
  return inb(COM1 + 5) & 0x20;
}


static void put_char(char c) {
  while (is_transmit_empty() == 0);
  outb(COM1, c);
}


static void serial_dev_write(void* _unused fsnode, char* buf, size_t n_bytes) {
  for (size_t i = 0; i < n_bytes; ++i) {
    put_char(buf[i]);
  }
}

static file_ops_t serial_fops = {
  .open = NULL,
  .write = serial_dev_write,
  .read = NULL,
};

void serial_init(void) {
  outb(COM1 + 1, 0x00);    // Disable interrupts
  outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
  outb(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
  outb(COM1 + 1, 0x00);    //                  (hi byte)
  outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
  outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
  outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
  outb(COM1 + 4, 0x1E);    // Set in loopback mode, test the serial chip
  outb(COM1 + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

  // Ensure serial is not faulty.
  if (inb(COM1 + 0) != 0xAE) {
    faulty = 1;
    return;
  }

  // Set normal operation mode.
  outb(COM1 + 4, 0x0F);
  register_chrdev("serial", &serial_fops);

}

void serial_write(const char* str) {
  if (faulty)
    return;

  for (const char* s = str; *s; ++s) {
    put_char(*s);
  }
}
