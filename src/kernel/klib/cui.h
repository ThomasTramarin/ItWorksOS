#ifndef CUI_H
#define CUI_H

#include "../vga.h"

#define cui_klog(fmt, ...)                                                     \
  do {                                                                         \
    cui_printf("[KERNEL] " fmt, ##__VA_ARGS__);                                \
  } while (0)

#define CUI_TAB_SIZE 4

/**
 * Kernel CUI (Character User Interface) module
 *
 * This module provides a layer of abstraction for character-based Input/Output
 */

struct cui_state {
  uint8_t cursor_x;
  uint8_t cursor_y;
  struct vga_cell_attr current_attr;
};

void cui_init(uint8_t default_fg, uint8_t default_bg, uint8_t default_blank);
void cui_clear(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

void cui_cursor_set_xy(uint8_t x, uint8_t y);
void cui_cursor_get_xy(uint8_t *x, uint8_t *y);

void cui_putc_at(char ch, uint8_t x, uint8_t y);
void cui_putc(char ch);
void cui_puts(const char *str);

void __attribute__((cdecl)) cui_printf(const char *fmt, ...);

#endif