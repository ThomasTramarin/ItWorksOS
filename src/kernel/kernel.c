#include "kernel.h"
#include "vga.h"

void kmain() {

  vga_init(VGA_COLOR_WHITE, VGA_COLOR_BLUE, VGA_BLINK_FALSE);
  vga_clear_screen();

  vga_puts("ItWorksOnMyHP-OS");

  while (1) {
    asm volatile("hlt");
  }
}
