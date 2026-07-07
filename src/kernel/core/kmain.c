#include <arch/x86/interrupts/exceptions.h>
#include <arch/x86/interrupts/idt.h>
#include <arch/x86/interrupts/pic.h>
#include <arch/x86/timer/pit.h>
#include <core/kmain.h>
#include <drivers/video/vga/vga.h>
#include <klib/cui.h>

void kmain(boot_info_t *info) {
  cui_init(VGA_COLOR_WHITE, VGA_COLOR_BLUE, 0);

  cui_klog("Boot info: %c%c%c%c (magic), 0x%x (boot_drive)\n", info->magic[0],
           info->magic[1], info->magic[2], info->magic[3], info->boot_drive);
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

  while (1) {
    asm volatile("hlt");
  }
}
