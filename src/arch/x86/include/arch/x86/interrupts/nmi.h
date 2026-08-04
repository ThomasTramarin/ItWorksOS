#ifndef ARCH_X86_INTERRUPTS_NMI_H
#define ARCH_X86_INTERRUPTS_NMI_H

#include <arch/x86/interrupts/frame.h>

/**
 * @brief x86 Non-Maskable Interrupt handler
 */
void x86_nmi_handler(struct x86_interrupt_frame *frame);

#endif