#include <klib/memory.h>
#include <klib/string.h>

size_t strlen(const char *str) {
  const char *s = str;

  while (*s)
    s++;

  return s - str;
}

int strcmp(const char *str1, const char *str2) {
  while (*str1 && (*str1 == *str2)) {
    str1++;
    str2++;
  }

  return (unsigned char)*str1 - (unsigned char)*str2;
}

int strncmp(const char *str1, const char *str2, size_t n) {
  while (n > 0 && *str1 && (*str1 == *str2)) {
    str1++;
    str2++;
    n--;
  }

  if (n == 0)
    return 0;

  return (unsigned char)*str1 - (unsigned char)*str2;
}

char *strcpy(char *dst, const char *src) {
  size_t len =
      strlen(src) + 1; // +1 because strlen does not count the null terminator

  memcpy(dst, src, len);

  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *ret = dst;

  // copy characters
  while (n && *src) {
    *dst++ = *src++;
    n--;
  }

  // fill with '\0'
  while (n--) {
    *dst++ = '\0';
  }

  return ret;
}

char *strchr(const char *str, int c) {
  while (*str) {
    if (*str == (char)c)
      return (char *)str;

    str++;
  }

  if ((char)c == '\0')
    return (char *)str;

  return NULL;
}

char *strrchr(const char *str, int c) {
  const char *last = NULL;

  while (*str) {
    if (*str == (char)c)
      last = str;

    str++;
  }

  if ((char)c == '\0')
    return (char *)str;

  return (char *)last;
}