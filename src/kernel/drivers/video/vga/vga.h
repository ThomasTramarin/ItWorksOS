#ifndef VGA_H
#define VGA_H
#include <base/stdbool.h>

/**
 * @file vga.h
 * @brief VGA text mode driver interface (80x25 characters)
 *
 * Design:
 * - Uses a software backbuffer instead of writing directly to VGA memory
 * - Uses a dirty rectangle system to optimize screen updates
 * - Supports basic hardware cursor control via VGA CRT controller ports
 * - All coordinates starts from 0
 * - Out-of-bounds writes are ignored
 * - Flush must be called to apply changes
 */

// VGA CRT Controller PORTS
#define VGA_INDEX_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5

// VGA CRT Controller REGISTERS
#define VGA_CURSOR_START_REG 0x0A
#define VGA_CURSOR_END_REG 0x0B
#define VGA_CURSOR_HIBYTE_POS_REG 0x0E
#define VGA_CURSOR_LOBYTE_POS_REG 0x0F

// VGA text mode dimensions
#define VGA_COLS 80
#define VGA_ROWS 25

// VGA memory address
#define VGA_MEM_ADDR 0xB8000

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

/**
 * VGA text mode attribute byte layout
 * - fg: foreground color (0-15)
 * - bg: background color (0-7)
 * - blink: blink flag (bit 7 of attribute)
 */
struct __attribute__((packed)) vga_cell_attr {
  uint8_t fg : 4;
  uint8_t bg : 3;
  uint8_t blink : 1;
};

// One character cell in VGA text mode (char + attribute)
struct __attribute__((packed)) vga_cell {
  uint8_t ch;
  struct vga_cell_attr attr;
};

/**
 * Represents the region of the screen that has changed.
 * Used to minimize writes to VGA memory during flush.
 *
 * x1 = VGA_COLS, y1 = VGA_ROWS, x2 = 0, y2 = 0 -> means "no dirty region"
 * x1 = 0, y1 = 0, x2 = VGA_COLS - 1, y2 = VGA_ROWS - 1 -> means "fullscreen
 * dirty"
 */
struct vga_dirty_rect {
  uint8_t x1, y1;
  uint8_t x2, y2;
};

void vga_init(void);

void vga_set_cell(uint8_t x, uint8_t y, struct vga_cell cell);
bool vga_get_cell(uint8_t x, uint8_t y, struct vga_cell *out);

void vga_flush(void);
void vga_scroll(struct vga_cell_attr blank_attr);

void vga_fill_rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                   struct vga_cell cell);

void vga_cursor_enable(void);
void vga_cursor_disable(void);

void vga_cursor_set_xy(uint8_t x, uint8_t y);
void vga_cursor_get_xy(uint8_t *x, uint8_t *y);
void vga_cursor_set_shape(uint8_t start_scanline, uint8_t end_scanline);

#endif