#ifndef ARCH_X86_CPU_CONTROL_H
#define ARCH_X86_CPU_CONTROL_H
#include <base/stdint.h>

/*
 * The CR3 register contains the physical address of the
 * Page Directory currently used by the CPU.
 */
void x86_cr3_write(uint32_t value);
uint32_t x86_cr3_read(void);

#endif