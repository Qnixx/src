#include <drivers/timer/rtc.h>
#include <arch/x86/io.h>

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

static inline uint8_t is_updating(void) {
  outb(CMOS_ADDR, 0x0A);
  return inb(CMOS_DATA) & (1 << 7);
}

static inline uint8_t readb(uint8_t reg) {
  outb(CMOS_ADDR, reg);
  return inb(CMOS_DATA);
}

static rtc_time_t __read_datetime(void) {
  while (is_updating());
  rtc_time_t datetime;

  uint8_t last_second = readb(0x00);
  uint8_t last_minute = readb(0x02);
  uint8_t last_hour = readb(0x04);
  uint8_t last_day = readb(0x07);
  uint8_t last_month = readb(0x08);
  uint8_t last_year = readb(0x09);
  uint8_t registerB;
  
  do {
    datetime.sec = last_second;
    datetime.min = last_minute;
    datetime.hour = last_hour;
    datetime.day = last_day;
    datetime.month = last_month;
    datetime.year = last_year;

    while (is_updating());
    last_second = readb(0x00);
    last_minute = readb(0x02);
    last_second = readb(0x00);
    last_minute = readb(0x02);
    last_hour = readb(0x04);
    last_day = readb(0x07);
    last_month = readb(0x08);
    last_year = readb(0x09);
  } while (
    datetime.sec != last_second || 
    datetime.min != last_minute || 
    datetime.hour != last_hour || 
    datetime.day != last_day || 
    datetime.month != last_month || 
    datetime.year != last_year);
  
  registerB = readb(0x0B);

  if (!(registerB & (1 << 2))) {
    datetime.sec = (datetime.sec & 0x0F) + ((datetime.sec / 16) * 10);
    datetime.min = (datetime.min & 0x0F) + ((datetime.min / 16) * 10);
    datetime.hour = ((datetime.hour & 0x0F) + (((datetime.hour & 0x70) / 16) * 10) ) | (datetime.hour & 0x80);
    datetime.day = (datetime.day & 0x0F) + ((datetime.day / 16) * 10);
    datetime.month = (datetime.month & 0x0F) + ((datetime.month / 16) * 10);
    datetime.year = (datetime.year & 0x0F) + ((datetime.year / 16) * 10);
  }

  return datetime;
}

rtc_time_t rtc_read_datetime(void) {
  // TODO: Use interrupts to update time instead of reading every time.
  return __read_datetime();
}
