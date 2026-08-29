#ifndef DRIVERS_IRQCHIP_I8259_H
#define DRIVERS_IRQCHIP_I8259_H

#include <base/stdint.h>

int32_t i8259_init(void);

struct irq_chip *i8259_get_chip(void);
struct irq_map *i8259_get_map(void);

#endif