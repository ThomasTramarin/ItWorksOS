#include "klib/cui.h"
#include <klib/printf.h>
#include <klib/ringbuf.h>
#include <log/printk.h>
#include <log/syslog.h>

int printk(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int ret = vprintk(fmt, args);

  va_end(args);
  return ret;
}

int vprintk(const char *fmt, va_list args) {
  char buffer[1024];

  int total_len = vsnprintf(buffer, sizeof(buffer), fmt, args);

  syslog_write(buffer);
  cui_puts(buffer); // print also to the screen

  return total_len;
}