#include "vga.h"
#include "io.h"

struct __attribute__((packed)) vga_driver {
  struct vga_cell *text_buffer;
  uint8_t cursorX;
  uint8_t cursorY;
  uint8_t defaultFg;
  uint8_t defaultBg;
  uint8_t defaultBlink;
};

static struct vga_driver vga_driver;

void vga_init(uint8_t defaultFg, uint8_t defaultBg, uint8_t defaultBlink) {

  vga_driver.text_buffer = (struct vga_cell *)0xB8000;
  vga_driver.defaultBg = defaultBg;
  vga_driver.defaultFg = defaultFg;
  vga_driver.cursorX = 0;
  vga_driver.cursorY = 0;
  vga_driver.defaultBlink = defaultBlink;

  vga_clear_screen();
  vga_disable_cursor();
}

void vga_clear_screen() {
  for (int x = 0; x < VGA_COLS; x++) {
    for (int y = 0; y < VGA_ROWS; y++) {
      vga_set_char(' ', x, y);
    }
  }
}

void vga_disable_cursor() {
  outb(PORT_VGA_INDEX, VGA_REG_CURSOR_START);
  outb(PORT_VGA_DATA, VGA_CURSOR_DISABLE);
}

void vga_set_cell(struct vga_cell *cell, uint32_t x, uint32_t y) {
  if (x >= VGA_COLS || y >= VGA_ROWS)
    return;

  vga_driver.text_buffer[(VGA_COLS * y) + x] = *cell;
}

void vga_set_char(char ch, uint32_t x, uint32_t y) {

  struct vga_cell default_cell = {
      .ch = ch,
      .attr =
          {
              .bg = vga_driver.defaultBg,
              .fg = vga_driver.defaultFg,
              .blink = vga_driver.defaultBlink,
          },
  };

  vga_set_cell(&default_cell, x, y);
}

void vga_putc(char ch) {

  switch (ch) {
  case '\n':
    vga_driver.cursorY++;
    vga_driver.cursorX = 0; // implicit CR
    return;
  case '\r':
    vga_driver.cursorX = 0;
    return;
  }

  vga_set_char(ch, vga_driver.cursorX, vga_driver.cursorY);

  vga_driver.cursorX++;

  if (vga_driver.cursorX >= VGA_COLS) {
    vga_driver.cursorX = 0;
    vga_driver.cursorY++;
  }
}

void vga_puts(const char *str) {
  while (*str != '\0') {
    vga_putc(*str);
    str++;
  }
}