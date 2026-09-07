#ifndef CONSOLE_CONSOLE_H
#define CONSOLE_CONSOLE_H

#include <base/bit.h>
#include <base/stdbool.h>
#include <base/stddef.h>

#define CONSOLE_TAB_SIZE 4

/**
 * @brief Console abstraction (text-based output interface)
 *
 * The current implementation does not support output history.
 * write() automatically handles scrolling, however lines that are not visible
 * in the current viewport are discarded.
 *
 * Currently only one console is supported. Once a console is registered through
 * console_register() function, that console becomes active and the kernel can
 * use console_* functions to control the active console (e.g. console_write(),
 * console_set_cursor()).
 */
struct console {
  /**
   * @brief Console name identifier
   */
  const char *name;

  /**
   * @brief Number of characters in a row
   */
  uint32_t width;

  /**
   * @brief Number of characters in a column
   */
  uint32_t height;

  /**
   * @brief The current curstor's x coordinate (which column)
   * The number starts from 0
   * Range values: [0, width)
   */
  uint32_t cursor_x;

  /**
   * @brief The current cursor's y coordinate (which row)
   * The number starts from 0
   * Range values: [0, height)
   */
  uint32_t cursor_y;

  /**
   * @brief Write a string to the output screen
   *
   * Writes the string starting from the current cursor position.
   *
   * This function automatically handles control characters (for example '\n' or
   * '\t', '\b') and scrolling up.
   *
   * @param con This console instance
   * @param buf Buffer to the string to write
   * @param len The number of bytes to write
   * @return KERR_OK on success, or a negative kernele error code
   */
  int32_t (*write)(struct console *con, const char *buf, size_t len);

  /**
   * @brief Clear screen
   * @param con This console instance
   * @return KERR_OK on success, or a negative kernele error code
   */
  int32_t (*clear)(struct console *con);

  /**
   * @brief Update cursor to specific position
   *
   *
   * @param con Pointer to this console instance
   * @param x Target column coordinate [0, width)
   * @param y Target row coordinate [0, height)
   * @return KERR_OK on success, or a negative kernel error code
   */
  int32_t (*set_cursor_position)(struct console *con, uint32_t x, uint32_t y);

  /**
   * @brief Enable and disable cursor
   *
   * @param con This console instance
   * @param visible true = visible, false = not visible
   * @return KERR_OK on success, or a negative kernel error code
   */
  int32_t (*set_cursor_visible)(struct console *con, bool visible);
};

int32_t console_register(struct console *con);
int32_t console_write(const char *buf, size_t len);
int32_t console_clear(void);
int32_t console_set_cursor_position(uint32_t x, uint32_t y);
int32_t console_set_cursor_visible(bool visible);

#endif