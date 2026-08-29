#include <irq/chip.h>
#include <irq/desc.h>
#include <irq/irq.h>
#include <irq/map.h>
#include <kernel/error.h>
#include <klib/bitmap.h>

/**
 * @brief Kernel virtual IRQ bitmap
 *
 * Each bit represents the state of a virq:
 *  0 = virq is free
 *  1 = virq is allocated (mapping hwirq_t -> irq_t exists)
 */
static struct bitmap irq_allocated;
static uint32_t irq_allocated_data[BITMAP_ELEMS(IRQ_MAX)];

int32_t irq_init(void) {
  bitmap_init(&irq_allocated, irq_allocated_data, IRQ_MAX);

  return KERR_OK;
}

int32_t irq_alloc_range(uint32_t n, irq_t *out) {
  if (n == 0 || !out)
    return -KERR_INVAL;

  if (n > IRQ_MAX)
    return -KERR_INVAL;

  size_t consecutive = 0;

  for (uint32_t i = 0; i < IRQ_MAX; i++) {
    if (bitmap_test(&irq_allocated, i)) {
      consecutive = 0;
      continue;
    }

    if (++consecutive == n) {
      irq_t first_irq = i - n + 1;

      for (size_t j = 0; j < n; j++) {
        bitmap_set(&irq_allocated, first_irq + j);
      }

      *out = first_irq;

      return KERR_OK;
    }
  }

  return -KERR_NOSPC;
}

int32_t irq_free_range(irq_t first_irq, uint32_t n) {
  if (n == 0)
    return -KERR_INVAL;

  if (first_irq >= IRQ_MAX || n > IRQ_MAX - first_irq)
    return -KERR_INVAL;

  for (size_t i = 0; i < n; i++) {
    if (!bitmap_test(&irq_allocated, i + first_irq))
      return -KERR_NOENT; /* The allocation does not exist */
  }

  for (size_t i = first_irq; i < first_irq + n; i++)
    bitmap_clear(&irq_allocated, i);

  return KERR_OK;
}

bool irq_is_allocated(irq_t irq) {
  if (irq >= IRQ_MAX)
    return false;

  return bitmap_test(&irq_allocated, irq);
}

int32_t irq_request(irq_t irq, irq_handler_t handler) {
  if (!handler || irq >= IRQ_MAX)
    return -KERR_INVAL;

  if (!irq_is_allocated(irq))
    return -KERR_NOENT;

  struct irq_desc *desc = irq_desc_get(irq);

  if (!desc->map || !desc->chip)
    return -KERR_NOENT;

  if (desc->handler)
    return -KERR_BUSY;

  desc->handler = handler;

  return KERR_OK;
}

int32_t irq_free(irq_t irq) {
  if (irq >= IRQ_MAX)
    return -KERR_INVAL;

  if (!irq_is_allocated(irq))
    return -KERR_NOENT;

  struct irq_desc *desc = irq_desc_get(irq);

  if (!desc->map || !desc->chip)
    return -KERR_NOENT;

  if (!desc->handler)
    return -KERR_NOENT;

  desc->handler = NULL;

  return KERR_OK;
}

int32_t irq_dispatch(struct irq_chip *chip, struct irq_map *map,
                     uint32_t vector) {

  hwirq_t hwirq;
  irq_t irq;
  struct irq_desc *desc;

  if (!chip || !map)
    return -KERR_INVAL;

  KERR_TRY(chip->handle_vector(vector, &hwirq));

  irq = irq_map_hwirq_to_irq(map, hwirq);

  if (irq == IRQ_INVALID)
    return -KERR_NOENT;

  desc = irq_desc_get(irq);

  if (!desc)
    return -KERR_NOENT;

  if (!desc->handler)
    return -KERR_NOENT;

  desc->handler();

  chip->eoi(desc);

  return KERR_OK;
}