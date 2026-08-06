#ifndef HAL_INTERRUPT_H
#define HAL_INTERRUPT_H

#include <base/stdbool.h>
#include <base/stdint.h>

void hal_interrupt_enable(void);
void hal_interrupt_disable(void);
bool hal_interrupt_enabled(void);

#endif