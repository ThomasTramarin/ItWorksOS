#include "kernel.h"
#include "idt.h"
#include "pic.h"
#include "vga.h"

void kmain() {

  vga_init(VGA_COLOR_WHITE, VGA_COLOR_BLUE, VGA_BLINK_FALSE);
  vga_clear_screen();

  vga_puts("ItWorksOnMyHP-OS\r\n");

  idt_init();
  vga_puts("IDT and ISR configurated.\r\n");

  pic_init();
  vga_puts("PIC configurated.\r\n");

  asm volatile("sti");
  vga_puts("Interrupts enabled.\r\n");

  while (1) {
    asm volatile("hlt");
  }
}
