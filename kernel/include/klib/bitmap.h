#ifndef KLIB_BITMAP_H
#define KLIB_BITMAP_H

#include <base/stdbool.h>
#include <base/stddef.h>

struct bitmap {
  uint32_t *data;
  size_t nbits;
  size_t nelems;
};

#define BITMAP_BITS_PER_ELEM 32

#define BITMAP_ELEMS(nbits)                                                    \
  (((nbits) + BITMAP_BITS_PER_ELEM - 1) / BITMAP_BITS_PER_ELEM)

#define BITMAP_SIZE(nbits) ((nbits) * sizeof(uint32_t))

void bitmap_init(struct bitmap *bm, uint32_t *buffer, size_t nbits);
void bitmap_set(struct bitmap *bm, size_t bitpos);
void bitmap_clear(struct bitmap *bm, size_t bitpos);

/**
 * @brief Returns true if the specified bit is set
 */
bool bitmap_test(const struct bitmap *bp, size_t bitpos);

/**
 * @brief Find the first cleared bit
 * Returns false if none exists
 */
bool bitmap_find_zero(const struct bitmap *bm, size_t *bitpos);

#endif