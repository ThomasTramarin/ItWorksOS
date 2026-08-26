#ifndef ARCH_X86_INTERRUPTS_EXCEPTION_H
#define ARCH_X86_INTERRUPTS_EXCEPTION_H

#include <arch/interrupts/frame.h>
#include <base/stdint.h>

void x86_exception_handler(struct x86_interrupt_frame *frame);

#endif