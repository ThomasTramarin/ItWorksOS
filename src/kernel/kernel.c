#include "kernel.h"
#include "exceptions.h"
#include "idt.h"
#include "klib/cui.h"
#include "klib/time.h"
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

  cui_klog("Interrupts enabled.\n");

  timer_init();
  cui_klog("PIT configurated.\n");

  asm volatile("sti");

  cui_puts("\nWelcome to ItWorksOnMyHP\n");

  vga_flush();

  uint64_t last_frame = 0;

  while (1) {

    uint64_t now = time_get_ticks();

    // VGA flush 30 FPS
    if (now - last_frame >= 33) {
      cui_printf("%d\n", now);
      vga_flush();
      last_frame = now;
    }

    asm volatile("hlt");
  }
}
