#include <arch/x86/interrupts/idt.h>
#include <arch/x86/interrupts/isr.h>

static struct idt_entry idt[256];
static struct idt_descriptor idt_ptr;

void idt_set_gate(uint8_t num, uintptr_t offset, uint16_t segment_selector,
                  uint8_t gate_type, uint8_t dpl, uint8_t present) {

  idt[num].offset_low = (uint16_t)(offset & 0xFFFF);
  idt[num].segment_selector = segment_selector;
  idt[num].reserved = 0;

  idt[num].flags = (present << 7) | (dpl << 5) | (0 << 4) | (gate_type & 0x0F);

  idt[num].offset_high = (uint16_t)((offset >> 16) & 0xFFFF);
}

void idt_init(void) {
  idt_ptr.size = (sizeof(struct idt_entry) * 256) - 1;
  idt_ptr.offset = (uintptr_t)&idt;

  isr_init();

  asm volatile("lidt (%0)" : : "r"(&idt_ptr));
}