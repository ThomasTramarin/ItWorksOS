#ifndef VGA_H
#define VGA_H
#include "types.h"

// VGA CRT Controller
#define PORT_VGA_INDEX 0x3D4
#define PORT_VGA_DATA 0x3D5

#define VGA_REG_CURSOR_START 0x0A
#define VGA_CURSOR_DISABLE 0x20

#define VGA_COLS 80
#define VGA_ROWS 25

// Standard dark colors
#define VGA_COLOR_BLACK 0x00
#define VGA_COLOR_BLUE 0x01
#define VGA_COLOR_GREEN 0x02
#define VGA_COLOR_CYAN 0x03
#define VGA_COLOR_RED 0x04
#define VGA_COLOR_MAGENTA 0x05
#define VGA_COLOR_BROWN 0x06
#define VGA_COLOR_LIGHT_GREY 0x07

// Bright / Light colors
#define VGA_COLOR_DARK_GREY 0x08
#define VGA_COLOR_LIGHT_BLUE 0x09
#define VGA_COLOR_LIGHT_GREEN 0x0A
#define VGA_COLOR_LIGHT_CYAN 0x0B
#define VGA_COLOR_LIGHT_RED 0x0C
#define VGA_COLOR_LIGHT_MAGENTA 0x0D
#define VGA_COLOR_YELLOW 0x0E
#define VGA_COLOR_WHITE 0x0F

#define VGA_BLINK_TRUE 1
#define VGA_BLINK_FALSE 0

// attributes
struct __attribute__((packed)) vga_cell_attr {
  uint8_t fg : 4;
  uint8_t bg : 3;
  uint8_t blink : 1;
};

struct __attribute__((packed)) vga_cell {
  char ch : 8;
  struct vga_cell_attr attr;
};

void vga_disable_cursor();

void vga_init(uint8_t defaultFg, uint8_t defaultBg, uint8_t defaultBlink);

void vga_clear_screen();

void vga_set_cell(struct vga_cell *cell, uint32_t x, uint32_t y);
void vga_set_char(char ch, uint32_t x, uint32_t y);
void vga_putc(char ch);
void vga_puts(const char *str);

#endif