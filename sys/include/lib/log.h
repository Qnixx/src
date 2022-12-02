#ifndef LOG_H_
#define LOG_H_

#include <lib/types.h>
#include <stdarg.h>

#define PRINTK_RED "\e[0;31m"
#define PRINTK_PANIC PRINTK_RED "kpanic: "

void printk(const char* fmt, ...);


#endif
