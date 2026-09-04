#include <arch/cpu/gdt.h>
#include <arch/interrupts/exception.h>
#include <arch/interrupts/idt.h>
#include <arch/interrupts/irq.h>
#include <arch/interrupts/isr.h>
#include <arch/interrupts/nmi.h>
#include <arch/interrupts/pic.h>
#include <base/sections.h>

extern void *x86_isr_stub_table[X86_IDT_ENTRIES];

void __init x86_isr_init(void) {

  struct x86_idt_gate gate = {
      .selector = X86_GDT_SELECTOR_KERN_CODE,
  };

  for (int i = 0; i < X86_IDT_ENTRIES; i++) {

    // exceptions
    if (i < 32) {

      uint8_t type = X86_IDT_GATE_INT_32;

      switch (i) {

      case 1: // #DB Debug Exception
      case 3: // #BP Breakpoint
      case 4: // #OF Overflow
        type = X86_IDT_GATE_TRAP_32;
        break;

      default:
        type = X86_IDT_GATE_INT_32;
        break;
      }

      gate.flags = X86_IDT_FLAG_BUILD(X86_IDT_DPL_RING0, type);

    } else if (i == 0x80) {

      // syscall
      gate.flags = X86_IDT_FLAG_BUILD(X86_IDT_DPL_RING3, X86_IDT_GATE_TRAP_32);

    } else {

      // hardware IRQ
      gate.flags = X86_IDT_FLAG_BUILD(X86_IDT_DPL_RING0, X86_IDT_GATE_INT_32);
    }

    gate.handler = (uintptr_t)x86_isr_stub_table[i];
    x86_idt_set_gate(i, &gate);
  }
}

void __attribute__((cdecl)) x86_isr_handler(struct x86_interrupt_frame *frame) {

  uint8_t vector = frame->int_no;

  // Non-Maskable Interrupt
  if (vector == 2) {
    x86_nmi_handler(frame);
    return;
  }

  // CPU exceptions
  if (vector < 32) {
    x86_exception_handler(frame);
    return;
  }

  x86_irq_handler(frame);
}