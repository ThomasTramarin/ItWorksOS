#ifndef ARCH_X86_GDT_H
#define ARCH_X86_GDT_H

/**
 * @brief GDT Segment Selector Offsets
 */
#define X86_GDT_SELECTOR_KERN_CODE 0x08
#define X86_GDT_SELECTOR_KERN_DATA 0x10
#define X86_GDT_SELECTOR_USER_CODE 0x18
#define X86_GDT_SELECTOR_USER_DATA 0x20
#define X86_GDT_SELECTOR_TSS 0x28

void x86_gdt_init(void);

#endif