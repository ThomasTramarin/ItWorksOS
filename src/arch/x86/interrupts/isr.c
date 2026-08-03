#include <arch/x86/cpu/gdt.h>
#include <arch/x86/interrupts/idt.h>
#include <arch/x86/interrupts/isr.h>
#include <arch/x86/interrupts/pic.h>

extern void *x86_isr_stub_table[X86_IDT_ENTRIES];

void __attribute__((cdecl)) x86_isr_init(void) {
  struct x86_idt_gate gate = {
      .selector = X86_GDT_SELECTOR_KERN_CODE,
      .flags = X86_IDT_FLAG_BUILD(X86_IDT_DPL_RING0, X86_IDT_GATE_INT_32),
  };

  for (int i = 0; i < X86_IDT_ENTRIES; i++) {
    gate.handler = (uintptr_t)x86_isr_stub_table[i];
    x86_idt_set_gate(i, &gate);
  }

  /* System Call */
  gate.handler = (uintptr_t)x86_isr_stub_table[0x80];
  gate.flags = X86_IDT_FLAG_BUILD(X86_IDT_DPL_RING3, X86_IDT_GATE_INT_32);
  x86_idt_set_gate(0x80, &gate);
}

void __attribute__((cdecl)) x86_isr_handler(struct x86_interrupt_frame *frame) {

  uint8_t vector = frame->int_no;

  if (vector < 32) {
    // x86_exception_handler(frame);
  }
}