#ifndef BASE_STDDEF_H
#define BASE_STDDEF_H

#include <base/stdint.h>

typedef int32_t ptrdiff_t;
typedef uint32_t size_t;

#define NULL ((void *)0)

#define offsetof(type, member) ((size_t)&(((type *)0)->member))

#define container_of(ptr, type, member)                                        \
  ((type *)((char *)(ptr) - offsetof(type, member)))

#endif