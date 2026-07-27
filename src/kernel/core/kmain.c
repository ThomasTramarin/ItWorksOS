#include "boot/boot_info.h"
#include <arch/x86/interrupts/exceptions.h>
#include <arch/x86/interrupts/idt.h>
#include <arch/x86/interrupts/pic.h>
#include <arch/x86/timer/pit.h>
#include <core/kmain.h>
#include <drivers/video/vga/vga.h>
#include <klib/cui.h>

void kmain(uint32_t magic, boot_info_t *info) {
  cui_init(VGA_COLOR_WHITE, VGA_COLOR_BLUE, 0);

  if (magic != BOOT_MAGIC) {
    cui_printf("Error: invalid magic. System Halted\n");
    goto halt;
  }

  cui_klog("Memory Map:\n");
  for (uint32_t i = 0; i < info->memory_map_count; i++) {
    boot_mem_map_entry_t entry = info->memory_map_ptr[i];

    uint32_t base_low = (uint32_t)entry.base;
    uint32_t len_low = (uint32_t)entry.length;

    cui_klog("\t%d. Base: 0x%x, Length: 0x%x, Type: %x, Attr: %x\n", i,
             base_low, len_low, entry.type, entry.attr);
  }

  idt_init();
  cui_klog("IDT and ISR configurated.\n");

  exceptions_init();
  cui_klog("Exception handling enabled.\n");

  pic_init();
  cui_klog("PIC configurated.\n");

  timer_init();
  cui_klog("PIT configurated.\n");

  asm volatile("sti");
  cui_klog("Interrupts enabled.\n");

  cui_puts("\nWelcome to ItWorksOnMyHP\n");

halt:
  while (1) {
    asm volatile("hlt");
  }
}
