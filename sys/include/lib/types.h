#ifndef TYPES_H_
#define TYPES_H_

#define NULL (void*)0ULL
#define true 1
#define false 0

#if defined(__x86_64__) || defined(__aarch64__)
typedef unsigned long long size_t;
typedef signed long long ssize_t;
#elif defined(__i386__)
typedef unsigned int size_t;
typedef signed int ssize_t;
#endif

typedef size_t uintptr_t;
typedef signed int int32_t;
typedef signed long long int64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned long long uint64_t;
typedef uint8_t bool;
typedef uint32_t wchar_t;

#endif
