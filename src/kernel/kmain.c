#include <arch/x86/pit.h>
#include <boot/boot_info.h>
#include <drivers/video/vga/vga.h>
#include <hal/cpu.h>
#include <hal/interrupt.h>
#include <kernel/arch.h>
#include <kernel/kmain.h>
#include <kernel/panic.h>
#include <kernel/printk.h>
#include <kernel/syslog.h>
#include <klib/cui.h>

void kmain(uint32_t magic, boot_info_t *info) {
  syslog_init();

  cui_init(VGA_COLOR_WHITE, VGA_COLOR_BLUE, 0);

  if (magic != BOOT_MAGIC) {
    panic("Invalid boot magic value");
  }

  pr_info("Memory Map:\n");
  for (uint32_t i = 0; i < info->memory_map_count; i++) {
    boot_mem_map_entry_t entry = info->memory_map_ptr[i];

    uint32_t base_low = (uint32_t)entry.base;
    uint32_t len_low = (uint32_t)entry.length;

    pr_info("%d. Base: 0x%x, Length: 0x%x, Type: %x, Attr: %x\n", i, base_low,
            len_low, entry.type, entry.attr);
  }

  arch_init();

  hal_interrupt_enable();

  cui_puts("Welcome to ItWorksOnMyHP");

  // CPU halt
  while (1) {
    hal_cpu_halt();
  }
}
