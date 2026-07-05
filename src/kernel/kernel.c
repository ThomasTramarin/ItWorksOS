#include "kernel.h"
#include "exceptions.h"
#include "idt.h"
#include "klib/cui.h"
#include "pic.h"
#include "timer.h"
#include "vga.h"

void kmain(void) {
  cui_init(VGA_COLOR_WHITE, VGA_COLOR_BLUE, 0);

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
