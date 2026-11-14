#include <console/console.h>
#include <hal/port.h>
#include <klib/printf.h>
#include <klib/ringbuf.h>
#include <log/printk.h>
#include <log/syslog.h>

int printk(enum log_level level, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int ret = vprintk(level, fmt, args);

  va_end(args);
  return ret;
}

int vprintk(enum log_level level, const char *fmt, va_list args) {
  // TODO: use a larger buffer

  char buffer[1024];

  int total_len = vsnprintf(buffer, sizeof(buffer), fmt, args);
  if (total_len < 0)
    return total_len;

  (void)syslog_write(level, buffer);

  return total_len;
}