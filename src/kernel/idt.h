#ifndef IDT_H
#define IDT_H
#include "types.h"

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
  uint16_t offset_low;       // bits 0-15
  uint16_t segment_selector; // bits 16-31
  uint8_t reserved;          // bits 32-39
  uint8_t flags;             // bits 40-47
  uint16_t offset_high;      // bits 48-63
};

struct __attribute__((packed)) idt_descriptor {
  uint16_t size;   // bits 0-15
  uint32_t offset; // bits 16-47
};

void idt_set_gate(uint8_t num, uintptr_t offset, uint16_t segment_selector,
                  uint8_t gate_type, uint8_t dpl, uint8_t present);

void idt_init(void);

#endif