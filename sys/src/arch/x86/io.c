#include <arch/x86/io.h>

uint8_t inb(uint16_t port) {
  uint8_t res;
  __asm__ __volatile__("in %%dx, %%al" : "=a" (res) : "d" (port));
  return res;
}

void outb(uint16_t port, uint8_t data) {
  __asm__ __volatile__("out %%al, %%dx" :: "a" (data), "d" (port));
}

void outw(uint16_t port, uint16_t data) {
  __asm__ __volatile__("outw %w0, %w1" :: "a" (data), "Nd" (port));
}

uint16_t inw(uint16_t port) {
  uint16_t data;
  __asm__ __volatile__("inw %w1, %w0" : "=a" (data) : "Nd" (port));
  return data;
}

void outl(uint16_t port, uint32_t data) {
  __asm__ __volatile__("outl %0, %w1" :: "a" (data), "Nd" (port));
}

uint32_t inl(uint16_t port) {
  uint32_t data;
  __asm__ __volatile__("inl %w1, %0" : "=a" (data) : "Nd" (port));
  return data;
}

void io_wait() {
  __asm__ __volatile__("out %%al, %%dx" :: "a" (0x0), "d" (0x80));
}
