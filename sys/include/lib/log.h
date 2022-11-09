#ifndef LOG_H_
#define LOG_H_

#include <stdarg.h>


void printk(const char* fmt, ...);

/*
 *  disable_video_logging() is used
 *  whenever you want to print information
 *  that will just fill up the screen 
 *  and is not needed on the screen 
 *  so you'd disable video logging
 *  and it'll only be in the serial
 *  log.
 *
 *
 */

void disable_video_logging(void);
void enable_video_logging(void);


#define PRINTK_SERIAL(fmt, ...)     \
  disable_video_logging();          \
  printk(fmt, __VA_ARGS__);         \
  enable_video_logging();


#endif
