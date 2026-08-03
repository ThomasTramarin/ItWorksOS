#include <arch/x86/interrupts/idt.h>
#include <base/bit.h>
#include <klib/memory.h>

struct x86_idt_raw_entry {
  uint16_t offset_low;       // offset[0:15]
  uint16_t segment_selector; // selector[0:15]
  uint8_t reserved;          // set to 0
  uint8_t flags;             // flags[0:7]
  uint16_t offset_high;      // offset[16:31]
} __attribute__((packed));

struct x86_idt_descriptor {
  uint16_t size;   // bits 0-15
  uint32_t offset; // bits 16-47
} __attribute__((packed));

static struct x86_idt_raw_entry idt[X86_IDT_ENTRIES];
static struct x86_idt_descriptor idt_desc;

void x86_idt_set_gate(uint8_t vector, struct x86_idt_gate *g) {

  idt[vector].offset_low = (uint16_t)(g->handler & 0xFFFF);
  idt[vector].segment_selector = g->selector;
  idt[vector].reserved = 0;

  idt[vector].flags = g->flags;

  idt[vector].offset_high = (uint16_t)((g->handler >> 16) & 0xFFFF);
}

static inline void x86_idt_desc_init(void) {
  idt_desc.size = sizeof(idt) - 1;
  idt_desc.offset = (uint32_t)idt;
}

void x86_idt_init(void) {
  memset(idt, 0, sizeof(idt));

  x86_idt_desc_init();
}

void x86_idt_load(void) {
  __asm__ volatile("lidtl (%0)" : : "r"(&idt_desc) : "memory");
}