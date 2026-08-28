#include <base/bit.h>
#include <base/limits.h>
#include <base/stdbool.h>
#include <klib/bitmap.h>

void bitmap_init(struct bitmap *bm, uint32_t *buffer, size_t nbits) {
  bm->data = buffer;
  bm->nbits = nbits;
}

void bitmap_set(struct bitmap *bm, size_t bitpos) {
  bm->data[bitpos / BITMAP_BITS_PER_ELEM] |= BIT(bitpos % BITMAP_BITS_PER_ELEM);
}

void bitmap_clear(struct bitmap *bm, size_t bitpos) {
  bm->data[bitpos / BITMAP_BITS_PER_ELEM] &=
      ~BIT(bitpos % BITMAP_BITS_PER_ELEM);
}

bool bitmap_test(const struct bitmap *bp, size_t bitpos) {
  return (bp->data[bitpos / BITMAP_BITS_PER_ELEM] &
          BIT(bitpos % BITMAP_BITS_PER_ELEM)) != 0;
}

size_t bitmap_size(size_t nbits) {
  size_t nelems = (nbits + BITMAP_BITS_PER_ELEM - 1) / BITMAP_BITS_PER_ELEM;
  return nelems * sizeof(uint32_t);
}

bool bitmap_find_zero(const struct bitmap *bm, size_t *bitpos) {
  for (size_t i = 0;
       i < ((bm->nbits + BITMAP_BITS_PER_ELEM - 1) / BITMAP_BITS_PER_ELEM);
       i++) {

    uint32_t free = ~bm->data[i];

    if (free == 0)
      continue;

    for (size_t bit = 0; bit < BITMAP_BITS_PER_ELEM; bit++) {
      if (free & BIT(bit)) {

        size_t pos = i * BITMAP_BITS_PER_ELEM + bit;

        if (pos >= bm->nbits)
          return false;

        if (bitpos)
          *bitpos = pos;
        return true;
      }
    }
  }

  return false; // the bitmap is full
}