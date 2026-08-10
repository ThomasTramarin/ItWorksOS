#include <arch/x86/cpu/control.h>

void x86_cr3_write(uint32_t value) {
  __asm__ __volatile__("mov %0, %%cr3" ::"r"(value));
}

uint32_t x86_cr3_read(void) {
  uint32_t value;
  __asm__ __volatile__("mov %%cr3, %0" : "=r"(value));
  return value;
}