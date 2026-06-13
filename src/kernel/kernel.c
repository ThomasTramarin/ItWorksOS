#include "kernel.h"
#include "idt.h"
#include "klib/time.h"
#include "pic.h"
#include "timer.h"
#include "vga.h"

void kmain(void) {
  vga_init(VGA_COLOR_WHITE, VGA_COLOR_BLUE, VGA_BLINK_FALSE);
  vga_clear_screen();
  idt_init();
  vga_puts("IDT and ISR configurated.\n");

  pic_init();
  vga_puts("PIC configurated.\n");

  asm volatile("sti");
  vga_puts("Interrupts enabled.\n");

  vga_puts("PIT configurated.\n");
  timer_init();

  vga_puts("\nWelcome to ItWorksOnMyHP\n");

  for (int i = 0; i < 80; i++) {
    time_sleep(100);
    vga_putc('.');
  }

  while (1) {
    asm volatile("hlt");
  }
}
