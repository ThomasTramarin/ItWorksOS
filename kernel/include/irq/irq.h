#ifndef IRQ_IRQ_H
#define IRQ_IRQ_H

#include <base/stdbool.h>
#include <base/stdint.h>

#define IRQ_INVALID ((irq_t) - 1)

// The number of virtual kernel IRQs (irq_t)
#define IRQ_MAX 256

typedef uint32_t hwirq_t;
typedef uint32_t irq_t;

struct irq_desc;
struct irq_chip;
struct irq_map;

typedef void (*irq_handler_t)(void);

/**
 * @brief Initialize the generic IRQ subsystem
 *
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t irq_init(void);

/**
 * @brief Allocate n contiguous kernel virtual IRQs
 *
 * @param n Number of kernel IRQs to allocate
 * @param out The first IRQ
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t irq_alloc_range(uint32_t n, irq_t *out);

/**
 * @brief Free n contiguous kernel virtual IRQs
 *
 * @param first_irq The first kernel IRQ number to free
 * @param n The number of IRQ to free
 * @return int32_t
 */
int32_t irq_free_range(irq_t first_irq, uint32_t n);

/**
 * @brief Check whether a kernel IRQ number is allocated or not
 *
 * @param irq The kernel virtual IRQ number
 * @return true if the kernel IRQ has already been allocated, false otherwise
 */
bool irq_is_allocated(irq_t irq);

/**
 * @brief Dispatch an incoming hardware IRQ
 *
 * This function is called by the architecture-specific IRQ handler.
 *
 * @param chip The IRQ controller that raised the interrupt
 * @param map The IRQ map representing the range of IRQs handled by the 'chip'
 * @param vector The architecture-specific interrupt number identifier
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t irq_dispatch(struct irq_chip *chip, struct irq_map *map,
                     uint32_t vector);
#endif