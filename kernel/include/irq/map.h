#ifndef IRQ_MAP_H
#define IRQ_MAP_H

#include <irq/irq.h>

/**
 * @brief Maps hardware interrupt numbers to kernel virtual IRQ numbers
 *
 * An IRQ map represents a hardware IRQ namespace and its corresponding
 * kernel virtual IRQ namespace.
 *
 * The current implementation supports only linear mappings, where a contiguous
 * range of hardware IRQs is mapped to a contiguous range of kernel IRQs.
 */
struct irq_map {
  const char *name;

  hwirq_t first_hwirq;
  irq_t first_irq;
  uint32_t count;
};

/**
 * @brief Initialize a linear IRQ map
 *
 * @param map The IRQ map to initialize
 * @param name The name of the map (debug purposes)
 * @param first_hwirq The first hardware IRQ represented by this map
 * @param count The number of hardware IRQs represented by this map
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t irq_map_init(struct irq_map *map, const char *name, hwirq_t first_hwirq,
                     uint32_t count);

/**
 * @brief Translate a hardware IRQ into a kernel virtual IRQ
 *
 * @param map IRQ map to use for the translation
 * @param hwirq Hardware IRQ to translate
 * @return A valid virtual IRQ number on success or IRQ_INVALID if map is NULL
 *         or hwirq is outside the hardware IRQ range represented by the map
 */
irq_t irq_map_hwirq_to_irq(struct irq_map *map, hwirq_t hwirq);

/**
 * @brief Destroy an IRQ map
 *
 * Releases the contiguous VIRQ range previously allocated by irq_map_init()
 *
 * @param map The map to destroy
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t irq_map_destroy(struct irq_map *map);

#endif