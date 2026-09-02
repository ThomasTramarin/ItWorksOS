#include <boot/boot_info.h>
#include <device/platform.h>
#include <hal/cpu.h>
#include <hal/interrupt.h>
#include <irq/irq.h>
#include <kernel/arch.h>
#include <kernel/kmain.h>
#include <kernel/panic.h>
#include <kernel/printk.h>
#include <kernel/syslog.h>
#include <klib/cui.h>
#include <mm/mm.h>

void kmain(uint32_t magic, paddr_t boot_info_phys) {

  struct boot_info *info = (struct boot_info *)PHYS_TO_VIRT(boot_info_phys);

  syslog_init();

  cui_init(VGA_COLOR_WHITE, VGA_COLOR_BLUE, 0);

  if (magic != BOOT_MAGIC) {
    panic("Invalid boot magic value");
  }

  const struct boot_mem_map_entry *map =
      (const struct boot_mem_map_entry *)PHYS_TO_VIRT(info->memory_map_phys);

  if (mm_init(map, info->memory_map_count) < 0) {
    panic("Failed to initialize the Memory Manager");
  }
  kheap_dump();

  arch_init();

  if (irq_init() < 0) {
    panic("Failed to initialize the IRQ generic subsystem");
  }

  if (arch_irqchip_init() < 0) {
    panic("Failed to initialize the architecture-specific IRQ chip");
  }

  if (platform_init() < 0) {
    panic("Failed to initialize platform bus");
  }

  hal_interrupt_enable();

  printk("Welcome to ItWorksOnMyHP\n");

  // CPU halt
  while (1) {
    hal_cpu_halt();
  }
}
