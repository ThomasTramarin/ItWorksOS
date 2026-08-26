#ifndef BASE_BIT_H
#define BASE_BIT_H

/**
 * @file bit.h
 *
 * @brief Macros to manipulate bits and bit fields.
 *
 * The MASK_* macros modify the destination value directly
 *
 * The FIELD_* macros do not modify the input value and return a transformed
 * value that must be assigned by the caller
 */

/**
 * @brief Creates a bit mask with a single bit set
 */
#define BIT(n) (1U << ((n)))

/**
 * @brief Sets bits specified by a mask.
 */
#define MASK_SET(v, mask) ((v) |= (mask))

/**
 * @brief Clears bits specified by a mask.
 */
#define MASK_CLEAR(v, mask) ((v) &= ~(mask))

/**
 * @brief Checks if any bit specified by a mask is set.
 */
#define MASK_TEST(v, mask) (((v) & (mask)) != 0)

/**
 * @brief Checks if all bits specified by a mask are set.
 */
#define MASK_TEST_ALL(v, mask) (((v) & (mask)) == (mask))

/**
 * @brief Extracts a bit field.
 */
#define FIELD_GET(v, mask, shift) (((v) & (mask)) >> (shift))

/**
 * @brief Updates a bit field.
 */
#define FIELD_SET(v, mask, shift, field)                                       \
  (((v) & ~(mask)) | (((field) << (shift)) & (mask)))

#endif