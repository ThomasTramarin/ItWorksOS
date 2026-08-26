#ifndef ARCH_X86_INTERRUPTS_FRAME_H
#define ARCH_X86_INTERRUPTS_FRAME_H

#include <base/stdint.h>

struct x86_interrupt_frame {
  uint32_t gs;
  uint32_t fs;
  uint32_t es;
  uint32_t ds;

  // pushed by PUSHAD
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp_dummy;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;

  uint32_t int_no;
  uint32_t err_code;

  // pushed by the CPU
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;

  // only valid when coming from lower privilege
  uint32_t user_esp;
  uint32_t user_ss;

} __attribute__((packed));

#endif