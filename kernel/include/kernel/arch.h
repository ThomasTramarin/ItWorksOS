#ifndef KERNEL_ARCH_H
#define KERNEL_ARCH_H

#include <base/stdint.h>

/**
 * @brief Initialize the hardware specific for the target architecture
 * Each architecture must implement this function inside src/arch/<arch>/arch.c
 */
void arch_init(void);

/**
 * @brief Initialize the Interrupt Controller(s)
 *
 * Each architecture must implement this function
 *
 * @return int32_t KERR_OK on success or a negative kernel error code
 */
int32_t arch_irqchip_init(void);

#endif