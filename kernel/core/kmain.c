#include <base/compiler.h>
#include <base/sections.h>
#include <boot/boot_info.h>
#include <device/platform.h>
#include <hal/cpu.h>
#include <hal/interrupt.h>
#include <irq/irq.h>
#include <kernel/arch.h>
#include <kernel/boot.h>
#include <kernel/error.h>
#include <kernel/initcall.h>
#include <kernel/kmain.h>
#include <log/panic.h>
#include <log/printk.h>
#include <log/syslog.h>
#include <mm/mm.h>
#include <process/pid.h>
#include <process/process.h>

void __noreturn kmain(uint32_t magic, paddr_t boot_info_phys) {

  syslog_init();

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

  pid_init();

  printk("Welcome to ItWorksOS\n");

  struct process *system_proc = process_create("system");
  if (KERR_PTR_IS_ERR(system_proc)) {
    panic("Failed to create system process: %d", KERR_PTR_ERR(system_proc));
  }

  struct boot_state *boot = boot_get_state();

  if (KERR_IS_ERR(process_load(system_proc, boot->system_image,
                               (size_t)boot->system_image_size))) {
    panic("Failed to load system process");
  }

  if (KERR_IS_ERR(process_prepare(system_proc))) {
    panic("Failed to prepare system process");
  }

  kheap_dump();

  // RUN SYSTEM PROCESS
  if (KERR_IS_ERR(vm_space_load(&system_proc->vm))) {
    panic("system: vm_space_load");
  }

  system_proc->state = PROCESS_RUNNING;

  if (KERR_IS_ERR(process_start(system_proc))) {
    panic("system: process_start");
  }

  while (true) {
    hal_cpu_halt();
  }
}
