#ifndef KERNEL_ARCH_H
#define KERNEL_ARCH_H

/**
 * @brief Initialize the hardware specific for the target architecture
 * Each architecture must implement this function inside src/arch/<arch>/arch.c
 */
void arch_init(void);

#endif