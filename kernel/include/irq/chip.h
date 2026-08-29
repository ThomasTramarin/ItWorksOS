#ifndef IRQ_CHIP_H
#define IRQ_CHIP_H

#include <base/stdint.h>
#include <irq/irq.h>

struct irq_desc;

/**
 * @brief Interrupt Controller
 *
 * This structure abstracts the internal operations of an interrupt controller
 * (such as PIC and APIC) by exposing consistent function pointers.
 */
struct irq_chip {
  const char *name;

  /*
   * This function will be called by the generic irq_dispatch() function.
   * It takes the architecture-specific vector number and returns in 'hwirq' the
   * hardware IRQ number of the interrupt controller that raised the IRQ.
   * The function returns KERR_OK if the IRQ is 'real' or a negative kernel
   * error code if the IRQ is invalid and should not continue to the normal IRQ
   * generic dispatch (e.g. spurious interrupts)
   */
  int32_t (*handle_vector)(uint32_t vector, hwirq_t *hwirq);

  /*
   * Enables the hardware IRQ line (unmask) represented by the descriptor
   */
  void (*enable)(struct irq_desc *desc);

  /*
   * Disables the hardware IRQ line (mask) represented by the descriptor
   */
  void (*disable)(struct irq_desc *desc);

  /*
   * Sends an End-Of-Interrupt signal to the controller registered in the
   * 'desc' descriptor
   */
  void (*eoi)(struct irq_desc *desc);
};

#endif