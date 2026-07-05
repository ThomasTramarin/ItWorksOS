#ifndef CONV_H
#define CONV_H

#include "types.h"

#define CONV_BASE_2 2
#define CONV_BASE_8 8
#define CONV_BASE_10 10
#define CONV_BASE_16 16

#define CONV_MAX_CHARS_BASE_2 33
#define CONV_MAX_CHARS_BASE_8 12
#define CONV_MAX_CHARS_BASE_10 12
#define CONV_MAX_CHARS_BASE_16 9

/**
 * @brief Converts an integer to its ASCII representation
 * @param value The integer
 * @param str Destination buffer. Must be at least CONV_ITOA_MAX_CHARS_BASE_X to
 * prevent buffer overflows
 * @param base  Numeric base (e.g., CONV_BASE_10, CONV_BASE_16).
 *
 * @note If base is 10 and value is negative, a '-' sign is added. All other
 * bases treat the value as raw bit patterns (unsigned).
 */
void conv_itoa(int32_t value, char *str, uint8_t base);

/**
 * @brief Converts ASCII to integer
 *
 * @param str ASCII string (must be null terminated)
 * @param base Numeric base (e.g., CONV_BASE_10, CONV_BASE_16).
 * @return int32_t
 */
int32_t conv_atoi(const char *str, uint8_t base);

#endif