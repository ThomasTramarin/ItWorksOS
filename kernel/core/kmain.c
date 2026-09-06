#include <base/compiler.h>
#include <base/sections.h>
#include <boot/boot_info.h>
#include <device/platform.h>
#include <hal/cpu.h>
#include <hal/interrupt.h>
#include <irq/irq.h>
#include <kernel/arch.h>
#include <kernel/boot.h>
#include <kernel/initcall.h>
#include <kernel/kmain.h>
#include <klib/cui.h>
#include <log/panic.h>
#include <log/printk.h>
#include <log/syslog.h>
#include <mm/mm.h>

void __noreturn kmain(uint32_t magic, paddr_t boot_info_phys) {

  syslog_init();
  cui_init(VGA_COLOR_WHITE, VGA_COLOR_BLUE, 0);

  struct boot_info *info = (struct boot_info *)PHYS_TO_VIRT(boot_info_phys);
  boot_init(info);

  if (magic != BOOT_MAGIC) {
    panic("Invalid boot magic value");
  }

  if (mm_init() < 0) {
    panic("Failed to initialize the Memory Manager");
  }

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

  initcalls_invoke_devdrv();

  kheap_dump();

  printk("Welcome to ItWorksOS\n");

  hal_interrupt_enable();

  // CPU halt
  while (1) {
    hal_cpu_halt();
  }
}
