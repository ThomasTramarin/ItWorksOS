#include "memory.h"

void *memset(void *dst, int val, size_t n) {
  uint8_t *d = dst;
  uint8_t v = (uint8_t)val;

  for (size_t i = 0; i < n; i++)
    d[i] = v;

  return dst;
}

void *memcpy(void *dst, const void *src, size_t n) {
  uint8_t *d = dst;
  const uint8_t *s = src;

  for (size_t i = 0; i < n; i++)
    d[i] = s[i];

  return dst;
}

int memcmp(const void *src1, const void *src2, size_t n) {
  const uint8_t *s1 = src1;
  const uint8_t *s2 = src2;

  for (size_t i = 0; i < n; i++) {
    if (s1[i] != s2[i]) {
      return s1[i] - s2[i];
    }
  }

  return 0;
}

void *memmove(void *dst, const void *src, size_t n) {
  uint8_t *d = dst;
  const uint8_t *s = src;

  if (d == s)
    return dst;

  if (d < s) {
    for (size_t i = 0; i < n; i++)
      d[i] = s[i];
  } else {
    for (size_t i = n; i > 0; i--)
      d[i - 1] = s[i - 1];
  }

  return dst;
}

void *memchr(const void *src, int val, size_t n) {
  const uint8_t *p = src;
  uint8_t v = (uint8_t)val;

  for (size_t i = 0; i < n; i++) {
    if (p[i] == v)
      return (void *)&p[i];
  }

  return NULL;
}