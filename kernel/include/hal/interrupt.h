#ifndef HAL_INTERRUPT_H
#define HAL_INTERRUPT_H

#include <base/stdbool.h>
#include <base/stdint.h>

void hal_interrupt_enable(void);
void hal_interrupt_disable(void);
bool hal_interrupt_enabled(void);

/**
 * @brief Disable hardware interrupts and save the previous state
 *
 * @return The previous interrupt state (true = enabled, false = disabled)
 */
bool hal_interrupt_save_disable(void);

/**
 * @brief Restore interrupts to the previous state
 *
 * @param state The value returned by hal_interrupt_save_disable()
 */
void hal_interrupt_restore(bool state);

#endif