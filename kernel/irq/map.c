#include <irq/irq.h>
#include <irq/map.h>
#include <kernel/error.h>
#include <klib/bitmap.h>
#include <klib/memory.h>

int32_t irq_map_init(struct irq_map *map, const char *name, hwirq_t first_hwirq,
                     uint32_t count) {

  if (!map || !name || count == 0)
    return -KERR_INVAL;

  /* Get a linear range of virq */
  irq_t irq;
  KERR_TRY(irq_alloc_range(count, &irq));

  map->first_irq = irq;
  map->name = name;
  map->first_hwirq = first_hwirq;
  map->count = count;

  return KERR_OK;
}

irq_t irq_map_hwirq_to_irq(struct irq_map *map, hwirq_t hwirq) {
  if (!map)
    return IRQ_INVALID;

  if (hwirq < map->first_hwirq || hwirq >= map->first_hwirq + map->count)
    return IRQ_INVALID;

  return map->first_irq + (hwirq - map->first_hwirq);
}

int32_t irq_map_destroy(struct irq_map *map) {
  if (!map)
    return -KERR_INVAL;

  KERR_TRY(irq_free_range(map->first_irq, map->count));

  return KERR_OK;
}