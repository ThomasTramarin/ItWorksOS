#ifndef KLIB_MEMORY_H
#define KLIB_MEMORY_H

#include <base/stddef.h>

/**
 * @brief Fill a memory region with a byte value.
 */
void *memset(void *dst, int val, size_t n);

/**
 * @brief Copy a memory region to another.
 *
 * Source and destination must not overlap.
 * Use memmove() when overlapping regions are possible.
 */
void *memcpy(void *dst, const void *src, size_t n);

/**
 * @brief Compare two memory regions byte by byte.
 *
 * Returns a value:
 *  < 0 if the first region is smaller
 *    0 if regions are equal
 *  > 0 if the first region is greater
 */
int memcmp(const void *src1, const void *src2, size_t n);

/**
 * @brief Move a memory region handling overlapping areas.
 *
 * Copies backwards when the destination overlaps
 * the source and is located after it.
 */
void *memmove(void *dst, const void *src, size_t n);

/**
 * @brief Search for a byte inside a memory region.
 *
 * Returns a pointer to the first occurrence,
 * or NULL if the byte is not found.
 */
void *memchr(const void *src, int val, size_t n);

#endif