#ifndef RTC_H_
#define RTC_H_

#include <lib/types.h>


typedef struct {
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
} rtc_time_t;


rtc_time_t rtc_read_datetime(void);
void rtc_init(void);


#endif
