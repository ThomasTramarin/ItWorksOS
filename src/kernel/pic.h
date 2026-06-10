#ifndef PIC_H
#define PIC_H

void pic_init(void);

void pic_enable_irq(unsigned char irq);
void pic_disable_irq(unsigned char irq);
void pic_set_all_irqs(unsigned char val);

void pic_send_eoi(unsigned char irq);

#endif