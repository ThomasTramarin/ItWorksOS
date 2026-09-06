#include <console/console.h>
#include <kernel/error.h>

/**
 * @brief Active console
 * Currently, the kernel supports only one console instance
 */
static struct console *active_con;

int32_t console_register(struct console *con) {
  if (!con)
    return -KERR_INVAL;

  if (active_con)
    return -KERR_BUSY;

  active_con = con;
  return KERR_OK;
}

int32_t console_write(const char *buf, size_t len) {
  if (!active_con)
    return -KERR_NODEV;
  return active_con->write(active_con, buf, len);
}

int32_t console_clear(void) {
  if (!active_con)
    return -KERR_NODEV;
  return active_con->clear(active_con);
}

int32_t console_set_cursor(uint32_t x, uint32_t y) {
  if (!active_con)
    return -KERR_NODEV;
  return active_con->set_cursor(active_con, x, y);
}