#ifndef ARCH_X86_IDT_H
#define ARCH_X86_IDT_H

#include <base/bit.h>
#include <base/stdint.h>

#define X86_IDT_ENTRIES 256

struct x86_idt_gate {
  uint32_t handler;
  uint16_t selector;
  uint8_t flags;
};

/*
 * IDT flags byte:
 *
 * 7    Present
 * 6-5  DPL
 * 5    Reserved (0)
 * 3-0  Gate Type
 */
#define X86_IDT_FLAG_PRESENT_MASK BIT(7)

#define X86_IDT_FLAG_DPL_SHIFT 5
#define X86_IDT_FLAG_DPL_MASK (BIT(5) | BIT(6))

#define X86_IDT_FLAG_TYPE_MASK 0x0F
#define X86_IDT_FLAG_TYPE_SHIFT 0

#define X86_IDT_FLAG_BUILD(dpl, type)                                          \
  (X86_IDT_FLAG_PRESENT_MASK |                                                 \
   FIELD_SET(0, X86_IDT_FLAG_DPL_MASK, X86_IDT_FLAG_DPL_SHIFT, dpl) |          \
   FIELD_SET(0, X86_IDT_FLAG_TYPE_MASK, X86_IDT_FLAG_TYPE_SHIFT, type))

typedef enum {
  X86_IDT_GATE_TASK = 0x5,
  X86_IDT_GATE_INT_32 = 0xE, // CPU clears EFLAGS.IF before entering the handler
  X86_IDT_GATE_TRAP_32 = 0xF // CPU does not clear EFLAGS.IF
} x86_idt_gate_type_t;

typedef enum {
  X86_IDT_DPL_RING0 = 0,
  X86_IDT_DPL_RING1 = 1,
  X86_IDT_DPL_RING2 = 2,
  X86_IDT_DPL_RING3 = 3
} x86_idt_dpl_t;

/**
 * @brief Initializes the Interrupt Descriptor Table
 *
 * Clears the IDT entries and initializes the IDT descriptor
 */
void x86_idt_init(void);

/**
 * @brief Loads the IDT into che CPU
 *
 * Executes the LIDT assembly instruction
 */
void x86_idt_load(void);

/**
 * @brief Sets an IDT gate entry
 */
void x86_idt_set_gate(uint8_t vector, struct x86_idt_gate *g);

#endif