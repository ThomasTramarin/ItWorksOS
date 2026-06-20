#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "isr.h"

void exceptions_handler(struct registers *regs);
void exceptions_init(void);

#endif