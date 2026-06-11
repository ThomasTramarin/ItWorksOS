#include "idt.h"
#include "isr.h"

static struct idt_entry idt[256];
static struct idt_descriptor idt_ptr;

void idt_set_gate(unsigned char num, unsigned int offset,
                  unsigned short segment_selector, unsigned char gate_type,
                  unsigned char dpl, unsigned char present) {

  idt[num].offset_low = offset & 0xFFFF;
  idt[num].segment_selector = segment_selector;
  idt[num].reserved = 0;

  idt[num].flags = (present << 7) | (dpl << 5) | (0 << 4) | (gate_type & 0x0F);

  idt[num].offset_high = (offset >> 16) & 0xFFFF;
}

void idt_init(void) {
  idt_ptr.size = (sizeof(struct idt_entry) * 256) - 1;
  idt_ptr.offset = (unsigned int)&idt;

  isr_init();

  asm volatile("lidt (%0)" : : "r"(&idt_ptr));
}