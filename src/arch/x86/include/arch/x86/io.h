#ifndef ARCH_X86_IO_H
#define ARCH_X86_IO_H

#include <base/stdint.h>

/* x86 port I/O (instructions 'in' and 'out') interface */

uint8_t x86_inb(uint16_t port);
uint16_t x86_inw(uint16_t port);
uint32_t x86_inl(uint16_t port);

void x86_outb(uint16_t port, uint8_t value);
void x86_outw(uint16_t port, uint16_t value);
void x86_outl(uint16_t port, uint32_t value);

#endif