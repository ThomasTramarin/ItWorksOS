#include "kernel.h"
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

  pic_init();
  cui_klog("PIC configurated.\n");

  cui_klog("Interrupts enabled.\n");

  cui_klog("PIT configurated.\n");
  timer_init();

  asm volatile("sti");

  cui_puts("\nWelcome to ItWorksOnMyHP\n");

  uint64_t last_frame = 0;

  while (1) {

    uint64_t now = time_get_ticks();

    // VGA flush 30 FPS
    if (now - last_frame >= 33) {
      vga_flush();
      last_frame = now;
    }

    asm volatile("hlt");
  }
}
