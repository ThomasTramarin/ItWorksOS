#include "cui.h"
#include "conv.h"
#include "va.h"

static struct cui_state state;

void cui_init(uint8_t default_fg, uint8_t default_bg, uint8_t default_blank) {
  state.current_attr.fg = default_fg;
  state.current_attr.bg = default_bg;
  state.current_attr.blink = default_blank;

  state.cursor_x = 0;
  state.cursor_y = 0;

  vga_init();

  cui_clear(0, 0, VGA_COLS - 1, VGA_ROWS - 1);
}

void cui_clear(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
  struct vga_cell blank = {
      .ch = ' ',
      .attr = state.current_attr,
  };

  vga_fill_rect(x1, y1, x2, y2, blank);
}

void cui_putc_at(char ch, uint8_t x, uint8_t y) {
  struct vga_cell cell = {
      .ch = ch,
      .attr = state.current_attr,
  };

  vga_set_cell(x, y, cell);
}

void cui_cursor_set_xy(uint8_t x, uint8_t y) {
  if (x < VGA_COLS)
    state.cursor_x = x;
  if (y < VGA_ROWS)
    state.cursor_y = y;
}

void cui_cursor_get_xy(uint8_t *x, uint8_t *y) {
  if (x)
    *x = state.cursor_x;
  if (y)
    *y = state.cursor_y;
}

static void cui_newline(void) {
  state.cursor_x = 0;
  state.cursor_y++;

  if (state.cursor_y >= VGA_ROWS) {
    vga_scroll(state.current_attr);
    state.cursor_y = VGA_ROWS - 1;
  }
}

static void cui_backspace(void) {
  if (state.cursor_x == 0 && state.cursor_y == 0)
    return;

  if (state.cursor_x > 0) {
    state.cursor_x--;
  } else {
    state.cursor_y--;
    state.cursor_x = VGA_COLS - 1;
  }

  cui_putc_at(' ', state.cursor_x, state.cursor_y);
}

static void cui_tab(void) {
  uint8_t mod = state.cursor_x % CUI_TAB_SIZE;
  uint8_t spaces = (mod == 0) ? 0 : (CUI_TAB_SIZE - mod);
  state.cursor_x += spaces;

  if (state.cursor_x >= VGA_COLS)
    cui_newline();
}

void cui_putc(char ch) {
  switch (ch) {
  case '\n':
    cui_newline();
    break;
  case '\r':
    state.cursor_x = 0;
    break;
  case '\b':
    cui_backspace();
    break;
  case '\t':
    cui_tab();
    break;
  case '\0':
    break;
  default:
    cui_putc_at(ch, state.cursor_x, state.cursor_y);
    state.cursor_x++;
    break;
  }

  if (state.cursor_x >= VGA_COLS)
    cui_newline();
}

void cui_puts(const char *str) {
  while (*str != '\0') {
    cui_putc(*str);
    str++;
  }
}

void __attribute__((cdecl)) cui_printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  char tmp[CONV_ITOA_MAX_CHARS_BASE_2]; // max ITOA digits

  for (int i = 0; fmt[i] != '\0'; i++) {
    if (fmt[i] == '%') {
      i++; // consume %

      switch (fmt[i]) {
      case 'd': {
        int32_t v = va_arg(args, int32_t);
        conv_itoa(v, tmp, CONV_ITOA_BASE_10);
        cui_puts(tmp);
        break;
      }

      case 'x': {
        uint32_t v = va_arg(args, uint32_t);
        conv_itoa(v, tmp, CONV_ITOA_BASE_16);
        cui_puts(tmp);
        break;
      }

      case 'o': {
        uint32_t v = va_arg(args, uint32_t);
        conv_itoa(v, tmp, CONV_ITOA_BASE_8);
        cui_puts(tmp);
        break;
      }

      case 'b': {
        uint32_t v = va_arg(args, uint32_t);
        conv_itoa(v, tmp, CONV_ITOA_BASE_2);
        cui_puts(tmp);
        break;
      }

      case 'p': {
        uint32_t v = va_arg(args, uint32_t);
        cui_puts("0x");
        conv_itoa(v, tmp, CONV_ITOA_BASE_16);
        cui_puts(tmp);
        break;
      }

      case 's': {
        char *s = va_arg(args, char *);
        cui_puts(s);
        break;
      }

      case 'c': {
        char ch = (char)va_arg(args, int);
        cui_putc(ch);
        break;
      }

      case '%': {
        cui_putc('%');
        break;
      }

      default: {
        cui_putc('%');
        cui_putc(fmt[i]);
        break;
      }
      }
    } else {
      cui_putc(fmt[i]);
    }
  }

  va_end(args);
}