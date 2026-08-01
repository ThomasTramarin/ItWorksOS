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