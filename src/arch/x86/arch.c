#include <arch/x86/cpu/cpuid.h>
#include <arch/x86/cpu/flags.h>
#include <arch/x86/cpu/gdt.h>
#include <arch/x86/interrupts/idt.h>
#include <arch/x86/interrupts/isr.h>
#include <arch/x86/interrupts/pic.h>
#include <kernel/arch.h>
#include <kernel/panic.h>

void arch_init(void) {
  x86_cli(); // disable interrupts
  x86_cld(); // clear direction flag

  if (!x86_cpu_detect()) {
    panic("CPUID is not supported");
  }

  x86_gdt_init();
  x86_idt_init();
  x86_isr_init();
  x86_pic_init();

  x86_idt_load();
}