#ifndef ARCH_X86_CPU_FLAGS_H
#define ARCH_X86_CPU_FLAGS_H

#include <base/bit.h>
#include <base/stdbool.h>
#include <base/stdint.h>

/**
 * @file flags.h
 *
 * @brief x86 EFLAGS register operations and flag control instructions.
 */

/*
 * EFLAGS bit masks
 */
#define X86_FLAGS_CF_MASK BIT(0)  // Carry Flag
#define X86_FLAGS_PF_MASK BIT(2)  // Parity Flag
#define X86_FLAGS_AF_MASK BIT(4)  // Auxiliary Carry Flag
#define X86_FLAGS_ZF_MASK BIT(6)  // Zero Flag
#define X86_FLAGS_SF_MASK BIT(7)  // Sign Flag
#define X86_FLAGS_TF_MASK BIT(8)  // Trap Flag
#define X86_FLAGS_IF_MASK BIT(9)  // Interrupt Enable Flag
#define X86_FLAGS_DF_MASK BIT(10) // Direction Flag
#define X86_FLAGS_OF_MASK BIT(11) // Overflow Flag

#define X86_FLAGS_IOPL_MASK (BIT(12) | BIT(13)) // I/O Privilege Level
#define X86_FLAGS_IOPL_SHIFT 12

#define X86_FLAGS_NT_MASK BIT(14) // Nested Task Flag

#define X86_FLAGS_RF_MASK BIT(16)  // Resume Flag
#define X86_FLAGS_VM_MASK BIT(17)  // Virtual 8086 Mode Flag
#define X86_FLAGS_AC_MASK BIT(18)  // Alignment Check Flag
#define X86_FLAGS_VIF_MASK BIT(19) // Virtual Interrupt Flag
#define X86_FLAGS_VIP_MASK BIT(20) // Virtual Interrupt Pending Flag
#define X86_FLAGS_ID_MASK BIT(21)  // CPUID Identification Flag

/*
 * Read/write EFLAGS
 */
uint32_t x86_eflags_read(void);
void x86_eflags_write(uint32_t eflags);

/*
 * Interrupt Flag (IF)
 */
void x86_cli(void);
void x86_sti(void);

bool x86_interrupts_enabled(void);

/*
 * Direction Flag (DF)
 */
void x86_cld(void);
void x86_std(void);

bool x86_direction_enabled(void);

/*
 * Trap Flag (TF)
 */

void x86_trap_enable(void);
void x86_trap_disable(void);

bool x86_trap_enabled(void);

#endif