#ifndef ARCH_X86_GDT_H
#define ARCH_X86_GDT_H

/**
 * @brief GDT Segment Selector Offsets
 */
#define X86_GDT_SELECTOR_KERN_CODE 0x08
#define X86_GDT_SELECTOR_KERN_DATA 0x10

void x86_gdt_init(void);

#endif