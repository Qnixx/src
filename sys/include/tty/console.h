#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <stdarg.h>
#include <lib/font.h>


void console_write(const char* fmt, va_list args);


#endif
