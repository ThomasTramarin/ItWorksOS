#ifndef KLIB_STRING_H
#define KLIB_STRING_H

#include <base/stddef.h>

/**
 * @brief Calculate the length of a null-terminated string.
 */
size_t strlen(const char *str);

/**
 * @brief Compare two null-terminated strings.
 *
 * @return
 *  < 0 if str1 is smaller
 *    0 if strings are equal
 *  > 0 if str1 is greater
 */
int strcmp(const char *str1, const char *str2);

/**
 * @brief Compare at most n characters of two strings.
 */
int strncmp(const char *str1, const char *str2, size_t n);

/**
 * @brief Copy a null-terminated string.
 *
 * Destination must have enough space.
 */
char *strcpy(char *dst, const char *src);

/**
 * @brief Copy at most n characters of a string.
 *
 * Destination is padded with null bytes if needed.
 */
char *strncpy(char *dst, const char *src, size_t n);

/**
 * @brief Find the first occurrence of a character in a string.
 */
char *strchr(const char *str, int c);

/**
 * @brief Find the last occurrence of a character in a string.
 */
char *strrchr(const char *str, int c);

#endif