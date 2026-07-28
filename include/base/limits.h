#ifndef BASE_LIMITS_H
#define BASE_LIMITS_H

/**
 * @file limits.h
 *
 * @brief Numeric limits for integer types
 */

#define CHAR_BIT 8
#define CHAR_MIN (-128)
#define CHAR_MAX 127
#define UCHAR_MAX 255U

#define INT8_MIN (-128)
#define INT8_MAX 127

#define INT16_MIN (-32768)
#define INT16_MAX 32767

#define INT32_MIN (-2147483647 - 1)
#define INT32_MAX 2147483647

#define UINT8_MAX 255U
#define UINT16_MAX 65535U
#define UINT32_MAX 4294967295U

#endif