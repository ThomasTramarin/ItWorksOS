#include <base/stdarg.h>
#include <base/stdbool.h>
#include <base/stddef.h>
#include <klib/printf.h>

struct printf_buffer {
  char *buf;
  size_t size;
  size_t pos;
  size_t total;
};

static void buffer_putc(struct printf_buffer *b, char c) {
  // write a maximum of size - 1 characters
  if (b->size > 0 && b->pos < b->size - 1) {
    b->buf[b->pos] = c;
    b->pos++;
  }

  b->total++;
}

static void buffer_puts(struct printf_buffer *b, const char *str) {
  while (*str) {
    buffer_putc(b, *str);
    str++;
  }
}

static void buffer_finish(struct printf_buffer *b) {
  if (b->size > 0) {
    if (b->pos < b->size)
      b->buf[b->pos] = '\0';
    else
      b->buf[b->size - 1] = '\0';
  }
}

// reverse a string
static void reverse(char *str, size_t len) {
  for (size_t i = 0; i < len / 2; i++) {
    char tmp = str[i];
    str[i] = str[len - i - 1];
    str[len - i - 1] = tmp;
  }
}

static void format_uint(struct printf_buffer *b, uint32_t value, uint32_t base,
                        bool uppercase) {
  char tmp[64];
  size_t i = 0;

  if (value == 0) {
    buffer_putc(b, '0');
    return;
  }

  while (value != 0) {
    uint32_t digit = value % base;
    if (digit < 10)
      tmp[i++] = '0' + digit;
    else
      tmp[i++] = (uppercase ? 'A' : 'a') + (digit - 10);

    value /= base;
  }

  reverse(tmp, i);

  for (size_t j = 0; j < i; j++)
    buffer_putc(b, tmp[j]);
}

static void format_int(struct printf_buffer *b, int32_t value) {
  if (value < 0) {
    buffer_putc(b, '-');

    /*
     * Avoid overflow with INT_MIN
     */
    uint32_t n = -(uint32_t)value;
    format_uint(b, n, 10, false);
  } else {
    format_uint(b, (uint32_t)value, 10, false);
  }
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
  struct printf_buffer b = {
      .buf = buf,
      .size = size,
      .pos = 0,
      .total = 0,
  };

  while (*fmt) {
    if (*fmt != '%') {
      buffer_putc(&b, *fmt);
      fmt++;
      continue;
    }

    // if the character is %
    fmt++; // consume %

    if (*fmt == '\0')
      break;

    switch (*fmt) {
    case '%': {
      buffer_putc(&b, '%');
      break;
    }
    case 'c': {
      char c = (char)va_arg(args, int);
      buffer_putc(&b, c);
      break;
    }
    case 's': {
      const char *s = va_arg(args, const char *);
      if (s)
        buffer_puts(&b, s);
      else
        buffer_puts(&b, "(null)");

      break;
    }
    case 'd':
    case 'i': {
      int value = va_arg(args, int);
      format_int(&b, value);
      break;
    }
    case 'u': {
      unsigned int value = va_arg(args, unsigned int);
      format_uint(&b, value, 10, false);
      break;
    }
    case 'x': {
      unsigned int value = va_arg(args, unsigned int);
      format_uint(&b, value, 16, false);
      break;
    }
    case 'X': {
      unsigned int value = va_arg(args, unsigned int);
      format_uint(&b, value, 16, true);
      break;
    }
    case 'o': {
      unsigned int value = va_arg(args, unsigned int);
      format_uint(&b, value, 8, false);
      break;
    }
    case 'p': {
      void *value = va_arg(args, void *);
      buffer_puts(&b, "0x");
      format_uint(&b, (uintptr_t)value, 16, false);
      break;
    }
    default: {
      buffer_putc(&b, '%');
      buffer_putc(&b, *fmt);
      break;
    }
    }

    fmt++;
  }

  buffer_finish(&b);

  return b.total;
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int ret = vsnprintf(buf, size, fmt, args);

  va_end(args);

  return ret;
}
