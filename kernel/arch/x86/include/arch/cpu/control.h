#ifndef ARCH_X86_CPU_CONTROL_H
#define ARCH_X86_CPU_CONTROL_H
#include <base/stdint.h>

/*
 * CR0
 */
#define X86_CR0_PE BIT(0)  /* Protection Enable */
#define X86_CR0_MP BIT(1)  /* Monitor Coprocessor */
#define X86_CR0_EM BIT(2)  /* Emulation */
#define X86_CR0_TS BIT(3)  /* Task Switched */
#define X86_CR0_ET BIT(4)  /* Extension Type */
#define X86_CR0_NE BIT(5)  /* Numeric Error */
#define X86_CR0_WP BIT(16) /* Write Protect */
#define X86_CR0_AM BIT(18) /* Alignment Mask */
#define X86_CR0_NW BIT(29) /* Not Write-through */
#define X86_CR0_CD BIT(30) /* Cache Disable */
#define X86_CR0_PG BIT(31) /* Paging */

void x86_cr0_write(uint32_t value);
uint32_t x86_cr0_read(void);

/*
 * CR2
 * Contains the linear address that caused the most recent page-fault exception
 */
uint32_t x86_cr2_read(void);

/*
 * The CR3 register contains the physical address of the
 * Page Directory currently used by the CPU and two flags
 */
#define X86_CR3_PWT BIT(3) /* Page-level Write-Through */
#define X86_CR3_PCD BIT(4) /* Page-level Cache Disable */
void x86_cr3_write(uint32_t value);
uint32_t x86_cr3_read(void);

/*
 * CR4
 */
#define X86_CR4_PSE BIT(4) /* Page Size Extensions (4 MiB pages) */
#define X86_CR4_PAE BIT(5) /* Physical Address Extension */

void x86_cr4_write(uint32_t value);
uint32_t x86_cr4_read(void);

#endif