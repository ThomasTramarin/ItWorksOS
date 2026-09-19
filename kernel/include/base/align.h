#ifndef BASE_ALIGN_H
#define BASE_ALIGN_H

/**
 * @brief Alignment helpers
 *
 * Works only if align is a power of two
 */
#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))

/**
 * @brief Test if the number x is a multiple of align
 */
#define ALIGN_TEST(x, align) ((x) % (align) == 0)
#endif