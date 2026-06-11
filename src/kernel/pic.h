#ifndef PIC_H
#define PIC_H
#include "types.h"

void pic_init(void);

void pic_enable_irq(uint8_t irq);
void pic_disable_irq(uint8_t irq);
void pic_set_all_irqs(bool enable);

void pic_send_eoi(uint8_t irq);

#endif