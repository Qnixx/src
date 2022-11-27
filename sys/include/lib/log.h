#ifndef LOG_H_
#define LOG_H_

#include <lib/types.h>
#include <limine.h>

#define COLOR_RED    "\033[1;31m"
#define COLOR_BLUE   "\033[1;34m"
#define COLOR_YELLOW "\033[1;93m"
#define COLOR_GREEN  "\033[1;32m"
#define COLOR_NONE   "\033[0m"

// Write a string to FB.
void puts(char* str);

// Print Formatted string
void printf(char* fmt, ...);

#endif
