#ifndef TIME_H_
#define TIME_H_

#include <lib/math.h>

#define SECONDS_PER_YEAR 31557600
#define SECONDS_PER_MONTH 2629800
#define SECONDS_PER_DAY 86400
#define SECONDS_PER_HOUR 3600
#define SECONDS_PER_MINUTE 60

#define GET_YEAR(stamp) (stamp / SECONDS_PER_YEAR)
#define GET_MONTH(stamp) ((stamp % SECONDS_PER_YEAR) / SECONDS_PER_MONTH)
#define GET_DAY(stamp) ((stamp % SECONDS_PER_MONTH) / SECONDS_PER_DAY)
#define GET_HOUR(stamp) ((stamp % SECONDS_PER_DAY) / SECONDS_PER_HOUR)
#define GET_MINUTE(stamp) ((stamp % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE)

const char* month_names[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

#endif
