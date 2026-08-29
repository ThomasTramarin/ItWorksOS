#ifndef IRQ_DESC_H
#define IRQ_DESC_H

#include <irq/irq.h>

struct irq_map;
struct irq_chip;

/**
 * @brief Interrupt Descriptor
 *
 * The kernel IRQ number is not stored in the descriptor itself. It is
 * represented by the index of the descriptor in the global IRQ descriptor table
 */
struct irq_desc {
  /*
   * Hardware IRQ number, the controller understands this value.
   * Used to get the hwirq from the virq.
   */
  hwirq_t hwirq;

  /*
   * IRQ map that owns this interrupt
   * Used to get the hwirq from the virq
   */
  struct irq_map *map;

  /*
   * Interrupt controller operations
   * Provides hardware-specific operations used to manage this IRQ
   */
  struct irq_chip *chip;

  /**
   * Intrrupt handler function called by the generic IRQ subsystem when this IRQ
   * is dispatched
   */
  irq_handler_t handler;
};

/**
 * @brief Configure an IRQ descriptor
 *
 * @param irq Kernel virtual IRQ
 * @param hwirq Hardware IRQ
 * @param map IRQ map containing the hardware IRQ
 * @param chip Interrupt controller
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t irq_desc_configure(irq_t irq, hwirq_t hwirq, struct irq_map *map,
                           struct irq_chip *chip);

/**
 * @brief Get an IRQ descriptor from the IRQ table
 *
 * @param irq Kernel virtual IRQ
 * @return IRQ descriptor pointer or NULL if the IRQ is invalid
 */
struct irq_desc *irq_desc_get(irq_t irq);

#endif