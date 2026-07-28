#ifndef KLIB_PRINTF_H
#define KLIB_PRINTF_H

#include <base/stdarg.h>
#include <base/stddef.h>

/**
 * @brief Format a string using a va_list.
 *
 * Returns the number of characters that would have been written.
 */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

/**
 * @brief Format a string into a buffer.
 *
 * Returns the number of characters that would have been written.
 */
int snprintf(char *buf, size_t size, const char *fmt, ...);

#endif