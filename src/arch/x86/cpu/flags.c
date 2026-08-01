#include "base/bit.h"
#include <arch/x86/cpu/flags.h>

uint32_t x86_eflags_read(void) {
  uint32_t flags;
  __asm__ __volatile__("pushfl; popl %0"
                       : "=r"(flags)); // read EFLAGS (32 bits)
  return flags;
}

void x86_eflags_write(uint32_t eflags) {
  __asm__ __volatile__("pushl %0; popfl" : : "r"(eflags) : "cc", "memory");
}

void x86_cli(void) { __asm__ __volatile__("cli" ::: "memory", "cc"); }
void x86_sti(void) { __asm__ __volatile__("sti" ::: "memory", "cc"); }

bool x86_interrupts_enabled(void) {
  uint32_t flags = x86_eflags_read();
  return MASK_TEST(flags, X86_FLAGS_IF_MASK);
}

void x86_cld(void) { __asm__ __volatile__("cld" ::: "cc"); }
void x86_std(void) { __asm__ __volatile__("std" ::: "cc"); }

bool x86_direction_enabled(void) {
  uint32_t flags = x86_eflags_read();
  return MASK_TEST(flags, X86_FLAGS_DF_MASK);
}

void x86_trap_enable(void) {
  uint32_t flags = x86_eflags_read();
  MASK_SET(flags, X86_FLAGS_TF_MASK);
  x86_eflags_write(flags);
}

void x86_trap_disable(void) {
  uint32_t flags = x86_eflags_read();
  MASK_CLEAR(flags, X86_FLAGS_TF_MASK);
  x86_eflags_write(flags);
}

bool x86_trap_enabled(void) {
  uint32_t flags = x86_eflags_read();
  return MASK_TEST(flags, X86_FLAGS_TF_MASK);
}