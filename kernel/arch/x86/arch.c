#include <arch/cpu/cpuid.h>
#include <arch/cpu/flags.h>
#include <arch/cpu/gdt.h>
#include <arch/interrupts/idt.h>
#include <arch/interrupts/isr.h>
#include <base/sections.h>
#include <kernel/arch.h>
#include <kernel/panic.h>

void __init arch_init(void) {
  x86_cli(); // disable interrupts
  x86_cld(); // clear direction flag

  if (!x86_cpu_detect()) {
    panic("CPUID is not supported");
  }

  x86_gdt_init();
  x86_idt_init();
  x86_isr_init();

  x86_idt_load();
}