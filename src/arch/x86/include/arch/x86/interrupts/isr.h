#ifndef ARCH_X86_INTERRUPTS_ISR_H
#define ARCH_X86_INTERRUPTS_ISR_H

#include <arch/x86/interrupts/frame.h>
#include <base/stdint.h>

/**
 * @brief Initialize the x86 ISR (Interrupt Service Routine) subsystem
 *
 * Sets all IDT gates as ring 0 interrupt gates.
 * Vector 0x80 is exposed to ring 3 for the syscall interface.
 */
void x86_isr_init(void);

/**
 * @brief Common interrupt dispatcher called from the assembly ISR stub
 *
 * This function receives the interrupt frame and dispatches the interrupt
 * to the handler registered for the corresponding vector
 */
void __attribute__((cdecl)) x86_isr_handler(struct x86_interrupt_frame *frame);

#endif