#ifndef ISR_H
#define ISR_H

#include "klib/types.h"

struct __attribute__((packed)) registers {
  uint32_t ds;
  uint32_t edi, esi, ebp, kern_esp, ebx, edx, ecx, eax;
  uint32_t int_no, err_code;
  uint32_t eip, cs, eflags;
};

typedef void (*isr_t)(struct registers *);

void __attribute__((cdecl)) isr_init(void);

void __attribute__((cdecl)) isr_handler(struct registers *regs);

void isr_register_handler(uint8_t n, isr_t handler);

#endif