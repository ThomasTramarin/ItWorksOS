#include <base/stddef.h>
#include <irq/desc.h>
#include <irq/irq.h>
#include <irq/map.h>
#include <kernel/error.h>

static struct irq_desc irq_descs[IRQ_MAX];

int32_t irq_desc_configure(irq_t irq, hwirq_t hwirq, struct irq_map *map,
                           struct irq_chip *chip) {
  if (irq >= IRQ_MAX || !map || !chip)
    return -KERR_INVAL;

  if (!irq_is_allocated(irq))
    return -KERR_NOENT;

  if (irq_descs[irq].map != NULL)
    return -KERR_BUSY;

  if (hwirq < map->first_hwirq || hwirq >= map->first_hwirq + map->count)
    return -KERR_INVAL;

  irq_descs[irq].hwirq = hwirq;
  irq_descs[irq].map = map;
  irq_descs[irq].chip = chip;

  return KERR_OK;
}

struct irq_desc *irq_desc_get(irq_t irq) {
  if (irq >= IRQ_MAX)
    return NULL;

  return &irq_descs[irq];
}