#include <console/console.h>
#include <hal/port.h>
#include <klib/printf.h>
#include <klib/ringbuf.h>
#include <log/printk.h>
#include <log/syslog.h>

/*
 * printk writes every message to the system log and prints them to the active
 * console when one is available.
 *
 * Before a console is initialized, messages are still stored in the system log
 * but are not displayed.
 */

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
  console_write(buffer, total_len);

  /*
   * Output the string through QEMU's debugcon device.
   *
   * TODO: Make debugcon output optional through kernel configuration.
   */
  for (int i = 0; i < total_len; i++) {
    hal_port_write8(0xE9, buffer[i]);
  }

  return total_len;
}