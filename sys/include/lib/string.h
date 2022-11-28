#ifndef STRING_H_
#define STRING_H_

#include <lib/types.h>

size_t kstrlen(const char* str);
bool kmemcmp(const char* str1, const char* str2, size_t n);
char* dec2str(size_t dec);
uint8_t* kstrncpy(uint8_t* dst, const uint8_t* src, const uint8_t len);
void kmemcpy(void* dst, const void* src, size_t len);
void kmemcpy16(void* dst, const void* src, size_t len);
void kmemcpy32(void* dst, const void* src, size_t len);
void kmemcpy64(void* dst, const void* src, size_t len);
uint8_t* hex2str(uint64_t hex_num);
void kmemzero(void* ptr, size_t n);
void kmemset(void* ptr, uint8_t data, size_t n);
void kmemset16(void* ptr, uint16_t data, size_t n);
void kmemset32(void* ptr, uint32_t data, size_t n);
void kmemset64(void* ptr, uint64_t data, size_t n);
uint8_t kstrcmp(const char* str1, const char* str2);
uint8_t kstrncmp(const char* str1, const char* str2, size_t n);
uint64_t hex2int(char* hex, size_t len);

#endif
