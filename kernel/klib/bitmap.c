#include <base/bit.h>
#include <base/limits.h>
#include <base/stdbool.h>
#include <klib/bitmap.h>

#define BITS_PER_ELEM 32

void bitmap_init(struct bitmap *bm, uint32_t *buffer, size_t nbits) {
  bm->data = buffer;
  bm->nbits = nbits;
}

void bitmap_set(struct bitmap *bm, size_t bitpos) {
  bm->data[bitpos / BITS_PER_ELEM] |= BIT(bitpos % BITS_PER_ELEM);
}

void bitmap_clear(struct bitmap *bm, size_t bitpos) {
  bm->data[bitpos / BITS_PER_ELEM] &= ~BIT(bitpos % BITS_PER_ELEM);
}

bool bitmap_test(const struct bitmap *bp, size_t bitpos) {
  return (bp->data[bitpos / BITS_PER_ELEM] & BIT(bitpos % BITS_PER_ELEM)) != 0;
}

bool bitmap_find_zero(const struct bitmap *bm, size_t *bitpos) {
  for (size_t i = 0; i < ((bm->nbits + BITS_PER_ELEM - 1) / BITS_PER_ELEM);
       i++) {

    uint32_t free = ~bm->data[i];

    if (free == 0)
      continue;

    for (size_t bit = 0; bit < BITS_PER_ELEM; bit++) {
      if (free & BIT(bit)) {

        size_t pos = i * BITS_PER_ELEM + bit;

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