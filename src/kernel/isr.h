#ifndef ISR_H
#define ISR_H

struct __attribute__((packed)) registers {
  unsigned int ds;
  unsigned int edi, esi, ebp, kern_esp, ebx, edx, ecx, eax;
  unsigned int int_no, err_code;
  unsigned int eip, cs, eflags;
};

typedef void (*isr_t)(struct registers *);

void __attribute__((cdecl)) isr_init(void);

void __attribute__((cdecl)) isr_handler(struct registers *regs);

#endif