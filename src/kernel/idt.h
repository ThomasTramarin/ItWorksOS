#ifndef IDT_H
#define IDT_H

/**
 * IDT entries scheme

 * 0 - 31 -> CPU exceptions
 * 32 - 47 -> IRQs
 * 48 - 255 -> Software Interrupts
 */

enum idt_gate_type {
  IDT_GATE_TASK = 0x5,
  IDT_GATE_INT_32 = 0xE, // CPU disables interrupts
  IDT_GATE_TRAP_32 = 0xF // CPU do not disable interrupts
};

enum idt_dpl {
  IDT_DPL_RING0 = 0,
  IDT_DPL_RING1 = 1,
  IDT_DPL_RING2 = 2,
  IDT_DPL_RING3 = 3
};

struct __attribute__((packed)) idt_entry {
  unsigned short offset_low;       // bits 0-15
  unsigned short segment_selector; // bits 16-31
  unsigned char reserved;          // bits 32-39
  unsigned char flags;             // bits 40-47
  unsigned short offset_high;      // bits 48-63
};

struct __attribute__((packed)) idt_descriptor {
  unsigned short size; // bits 0-15
  unsigned int offset; // bits 16-47
};

void idt_set_gate(unsigned char num, unsigned int offset,
                  unsigned short segment_selector, unsigned char gate_type,
                  unsigned char dpl, unsigned char present);

void idt_init(void);

#endif