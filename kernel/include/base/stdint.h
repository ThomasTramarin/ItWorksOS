#ifndef BASE_STDINT_H
#define BASE_STDINT_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

// integer types capable of holding pointers (32-bit)
typedef uint32_t uintptr_t;
typedef int32_t intptr_t;

// Physical and virtual address types
typedef uint64_t paddr_t;
typedef uintptr_t vaddr_t;

#endif