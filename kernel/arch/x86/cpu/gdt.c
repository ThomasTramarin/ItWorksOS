#include <arch/cpu/gdt.h>
#include <base/bit.h>
#include <base/sections.h>

#include <base/stdint.h>
#include <klib/memory.h>

/**
 * @brief GDT raw entry fields
 *
 * Limit (20-bit): the size of the segment
 *  the scale depends on the Granularity flag
 *
 */
struct x86_gdt_raw_entry {
  uint16_t limit_low;       // limit[0:15]
  uint16_t base_low;        // base[0:15]
  uint8_t base_mid;         // base[16:23]
  uint8_t access_byte;      // access_byte[0:7]
  uint8_t limit_high_flags; // limit_high[16:19] (low nibble), flags[0:3] (high
                            // nibble)
  uint8_t base_high;        // base[24:31]
} __attribute__((packed));

/**
 * @brief GDT logic entry
 * Limit (20-bit): maximum addressable unit (1 byte units/4KiB units depending
 *                 on Granularity (G) flag)
 * Base (32-bit):  linear starting segment address
 * Access (8-bit): X86_GDT_ACCESS_BYTE_*
 * Flags (4-bit): X86_GDT_FLAG_*
 */
struct x86_gdt_entry {
  uint32_t limit;
  uint32_t base;
  uint8_t access;
  uint8_t flags;
};

/**
 * @brief Granularity Flag
 * 0: byte granularity (limit is in 1 byte blocks)
 * 1: page granularity (limit is in 4KiB blocks)
 */
#define X86_GDT_FLAG_G_MASK BIT(3)

/**
 * @brief Size flag
 * 0: 16-bit protected mode segment
 * 1: 32-bit protected mode segment
 */
#define X86_GDT_FLAG_DB_MASK BIT(2)

/**
 * @brief Long mode code flag
 * 0: any other type of segment
 * 1: 64-bit code segment, DB should be clear
 */
#define X86_GDT_FLAG_L_MASK BIT(1)

#define X86_GDT_FLAG_RESERVED_MASK BIT(0)

/**
 * @brief Present bit
 * 0: invalid segment
 * 1: valid segment
 */
#define X86_GDT_ACCESS_BYTE_P_MASK BIT(7)

/**
 * @brief DPL (Description Proviledge Level) field
 * CPU privilege level of the segment
 * Values: 00 (highest privilege), 01, 10, 11 (lowest privilege)
 */
#define X86_GDT_ACCESS_BYTE_DPL_MASK (BIT(6) | BIT(5))

/**
 * @brief Descriptor type bit
 * 0: system segment
 * 1: code/data segment
 */
#define X86_GDT_ACCESS_BYTE_S_MASK BIT(4)

/**
 * @brief Executable bit
 * 0: data segment
 * 1: code segment
 */
#define X86_GDT_ACCESS_BYTE_E_MASK BIT(3)

/**
 * @brief Direction bit/Conforming bit
 * Data selectors (direction bit):
 *  0: the segment grows up
 *  1: the segment grows down
 * Code selectors (conforming bit):
 *  0: code can only be executed when CPL == DPL
 *  1: code can be executed from CPL >= DPL (CPL stays unchanged)
 */
#define X86_GDT_ACCESS_BYTE_DC_MASK BIT(2)

/**
 * @brief Readable bit/Writable bit
 * Data segments (writable bit):
 *  0: write is not allowed
 *  1: write is allowed
 * Code segments (readable bit):
 *  0: read is not allowed
 *  1: read is allowed
 */
#define X86_GDT_ACCESS_BYTE_RW_MASK BIT(1)

/**
 * @brief Accessed bit
 * 0: segment not accessed yet. CPU writes 1 on first use (causes #PF if GDT is
 * read-only)
 * 1: segment already accessed. Keeps GDT read-only safe
 * (recommended)
 */
#define X86_GDT_ACCESS_BYTE_A_MASK BIT(0)

struct x86_gdt_descriptor {
  uint16_t size;
  uint32_t offset;
} __attribute__((packed));

/* Null descriptor, Kernel Code and Data*/
#define X86_GDT_ENTRIES 3

static struct x86_gdt_raw_entry gdt[X86_GDT_ENTRIES];
static struct x86_gdt_descriptor gdt_descriptor;

static void x86_gdt_set_entry(int index, struct x86_gdt_entry *e) {
  gdt[index].base_low = (uint16_t)(e->base & 0xFFFF);
  gdt[index].base_mid = (uint8_t)((e->base >> 16) & 0xFF);
  gdt[index].base_high = (uint8_t)((e->base >> 24) & 0xFF);

  gdt[index].limit_low = (uint16_t)(e->limit & 0xFFFF);
  gdt[index].limit_high_flags = (uint8_t)((e->limit >> 16) & 0x0F);
  gdt[index].limit_high_flags |= (uint8_t)((e->flags << 4) & 0xF0);

  gdt[index].access_byte = e->access;
}

/* assembly function */
extern void x86_gdt_flush(struct x86_gdt_descriptor *gdtr);

void __init x86_gdt_init(void) {
  gdt_descriptor.size = sizeof(gdt) - 1;
  gdt_descriptor.offset = (uint32_t)gdt;

  memset(gdt, 0, sizeof(gdt));

  struct x86_gdt_entry null_descr = {0};
  struct x86_gdt_entry kern_code = {0};
  struct x86_gdt_entry kern_data = {0};

  // Null descriptor
  x86_gdt_set_entry(0, &null_descr);

  // Kernel Code Descriptor (Flat 4GB, Ring 0, Read/Execute)
  kern_code.base = 0;
  kern_code.limit = 0xFFFFF;
  kern_code.access = X86_GDT_ACCESS_BYTE_P_MASK | X86_GDT_ACCESS_BYTE_S_MASK |
                     X86_GDT_ACCESS_BYTE_E_MASK | X86_GDT_ACCESS_BYTE_RW_MASK |
                     X86_GDT_ACCESS_BYTE_A_MASK;
  kern_code.flags = X86_GDT_FLAG_G_MASK | X86_GDT_FLAG_DB_MASK;
  x86_gdt_set_entry(1, &kern_code);

  // Kernel Data Descriptor (Flat 4GB, Ring 0, Read/Write)
  kern_data.base = 0;
  kern_data.limit = 0xFFFFF;
  kern_data.access = X86_GDT_ACCESS_BYTE_P_MASK | X86_GDT_ACCESS_BYTE_S_MASK |
                     X86_GDT_ACCESS_BYTE_RW_MASK | X86_GDT_ACCESS_BYTE_A_MASK;
  kern_data.flags = X86_GDT_FLAG_G_MASK | X86_GDT_FLAG_DB_MASK;
  x86_gdt_set_entry(2, &kern_data);

  x86_gdt_flush(&gdt_descriptor);
}
