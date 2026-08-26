#ifndef ARCH_X86_INTERRUPTS_IRQ_H
#define ARCH_X86_INTERRUPTS_IRQ_H

#include <arch/interrupts/frame.h>

void x86_irq_handler(struct x86_interrupt_frame *frame);

#endif