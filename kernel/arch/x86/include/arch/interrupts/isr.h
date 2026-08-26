#ifndef ARCH_X86_INTERRUPTS_ISR_H
#define ARCH_X86_INTERRUPTS_ISR_H

#include <arch/interrupts/frame.h>
#include <base/stdint.h>

/**
 * @brief Initialize the x86 ISR (Interrupt Service Routine) subsystem
 *
 * Sets all IDT gates as ring 0 interrupt gates except:
 *  - 0x80: syscall gate (ring3, trap gate)
 *  - #DB, #BP, #OF: (ring0, trap gate)
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