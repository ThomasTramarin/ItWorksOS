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
#include <mm/pmm.h>

void kmain(uint32_t magic, paddr_t boot_info_phys) {

  // virtual address
  struct boot_info *info = (struct boot_info *)(boot_info_phys + 0xC0000000);

  syslog_init();

  cui_init(VGA_COLOR_WHITE, VGA_COLOR_BLUE, 0);

  if (magic != BOOT_MAGIC) {
    panic("Invalid boot magic value");
  }

  // virtual address
  const struct boot_mem_map_entry *map =
      (const struct boot_mem_map_entry *)(info->memory_map_phys + 0xC0000000);

  if (pmm_init(map, info->memory_map_count) < 0) {
    panic("Failed to initialize PMM");
  }

  arch_init();

  hal_interrupt_enable();

  cui_puts("Welcome to ItWorksOnMyHP");

  // CPU halt
  while (1) {
    hal_cpu_halt();
  }
}
