#include <lib/string.h>

size_t strlen(const char* str) {
    const char* s2 = str;
    while(*s2) s2++;

    return (s2 - str);
}
