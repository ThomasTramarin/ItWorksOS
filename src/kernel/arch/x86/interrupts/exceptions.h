#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <arch/x86/interrupts/isr.h>

void exceptions_handler(struct registers *regs);
void exceptions_init(void);

#endif