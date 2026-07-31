#ifndef PIC_H
#define PIC_H
#include <base/stdbool.h>

void pic_init(void);

void pic_enable_irq(uint8_t irq);
void pic_disable_irq(uint8_t irq);
void pic_set_all_irqs(bool enable);

void pic_send_eoi(uint8_t irq);

/**
 * @brief Reads the In-Service Register (ISR) of the selected PIC
 *
 * @param slave true to read Slave PIC, false to read Master PIC
 * @return uint8_t The 8-bit value of IS register
 */
uint8_t pic_read_reg_isr(bool slave);

#endif