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

#define INT64_MIN (-9223372036854775807LL - 1)
#define INT64_MAX 9223372036854775807LL
#define UINT64_MAX 18446744073709551615ULL

#define INTPTR_MIN INT32_MIN
#define INTPTR_MAX INT32_MAX

#define UINTPTR_MAX UINT32_MAX

#endif