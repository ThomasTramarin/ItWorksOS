#include "kernel.h"
#include "idt.h"
#include "pic.h"
#include "vga.h"

void kmain(void) {

  vga_init(VGA_COLOR_WHITE, VGA_COLOR_BLUE, VGA_BLINK_FALSE);
  vga_clear_screen();

  vga_puts("ItWorksOnMyHP-OS\n");

  idt_init();
  vga_puts("IDT and ISR configurated.\n");

  pic_init();
  vga_puts("PIC configurated.\n");

  asm volatile("sti");
  vga_puts("Interrupts enabled.\n");

  while (1) {
    asm volatile("hlt");
  }
}
