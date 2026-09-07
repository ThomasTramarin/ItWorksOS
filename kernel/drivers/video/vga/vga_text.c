#include <base/bit.h>
#include <base/sections.h>
#include <base/stddef.h>
#include <console/console.h>
#include <device/platform.h>
#include <device/resource.h>
#include <hal/port.h>
#include <kernel/error.h>
#include <kernel/initcall.h>
#include <mm/kheap.h>
#include <mm/layout.h>

#define VGA_TEXT_80X25_SIZE (80 * 25 * sizeof(uint16_t))

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

#define VGA_CELL(ch, fg, bg, blink)                                            \
  ((uint16_t)(uint8_t)(ch) | ((uint16_t)(fg) << 8) | ((uint16_t)(bg) << 12) |  \
   ((uint16_t)(blink) << 15))

#define VGA_CELL_CLEAR                                                         \
  VGA_CELL(' ', VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK, VGA_BLINK_FALSE)

#define VGA_INDEX_PORT_OFFSET 0 // CRTC Index
#define VGA_DATA_PORT_OFFSET 1  // CRTC Data

/*
 * Cell layout (16-bit):
 *  0-7:   CP437 character code
 *  8-11:  Foregroung color
 *  12-14: Background color
 *  15:    Blink
 */

struct vga_text_priv {
  struct console con;
  uint16_t *buffer; /* VGA text MMIO buffer */

  uint32_t port_base;
};

/* Update VGA hardware cursor position to match the logic cursor location */
static inline void vga_text_update_cursor(struct vga_text_priv *dev_priv,
                                          struct console *con) {
  uint16_t position = con->cursor_y * con->width + con->cursor_x;
  /*
   * Register 0x0E = cursor location high (higher 8 bits)
   * Register 0x0F = cursor location low (lower 8 bits)
   */
  hal_port_write8(dev_priv->port_base + VGA_INDEX_PORT_OFFSET, 0x0F);
  hal_port_write8(dev_priv->port_base + VGA_DATA_PORT_OFFSET,
                  (uint8_t)(position & 0xFF));

  hal_port_write8(dev_priv->port_base + VGA_INDEX_PORT_OFFSET, 0x0E);
  hal_port_write8(dev_priv->port_base + VGA_DATA_PORT_OFFSET,
                  (uint8_t)((position >> 8) & 0xFF));
}

static int32_t vga_text_console_write(struct console *con, const char *buf,
                                      size_t len) {
  if (!con || !buf)
    return -KERR_INVAL;

  struct vga_text_priv *dev_priv = container_of(con, struct vga_text_priv, con);

  for (size_t i = 0; i < len; i++) {
    char c = buf[i];

    switch (c) {
    case '\n': {
      con->cursor_x = 0;
      con->cursor_y++;
      break;
    }
    case '\r': {
      con->cursor_x = 0;
      break;
    }
    case '\t': {
      uint32_t spaces = CONSOLE_TAB_SIZE - (con->cursor_x % CONSOLE_TAB_SIZE);
      con->cursor_x += spaces;
      break;
    }
    case '\b': {
      if (con->cursor_x > 0) {
        con->cursor_x--;
      } else if (con->cursor_y > 0) {
        con->cursor_y--;
        con->cursor_x = con->width - 1;
      } else {
        break;
      }

      dev_priv->buffer[con->cursor_y * con->width + con->cursor_x] =
          VGA_CELL_CLEAR;

      break;
    }
    default: {
      dev_priv->buffer[con->cursor_y * con->width + con->cursor_x] =
          VGA_CELL(c, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK, VGA_BLINK_FALSE);
      con->cursor_x++;
      break;
    }
    }

    // Next line
    if (con->cursor_x >= con->width) {
      con->cursor_x = 0;
      con->cursor_y++;
    }

    // Scroll
    if (con->cursor_y >= con->height) {
      for (uint32_t y = 1; y < con->height; y++) {
        for (uint32_t x = 0; x < con->width; x++) {
          dev_priv->buffer[(y - 1) * con->width + x] =
              dev_priv->buffer[y * con->width + x];
        }
      }

      for (uint32_t x = 0; x < con->width; x++) {
        dev_priv->buffer[(con->height - 1) * con->width + x] = VGA_CELL_CLEAR;
      }

      con->cursor_y = con->height - 1;
    }
  }

  vga_text_update_cursor(dev_priv, con);

  return KERR_OK;
}

static int32_t vga_text_console_clear(struct console *con) {
  if (!con)
    return -KERR_INVAL;

  struct vga_text_priv *dev_priv = container_of(con, struct vga_text_priv, con);

  for (uint16_t *ptr = dev_priv->buffer; ptr < dev_priv->buffer + 80 * 25;
       ptr++) {
    *ptr = VGA_CELL_CLEAR;
  }

  con->cursor_x = 0;
  con->cursor_y = 0;

  vga_text_update_cursor(dev_priv, con);

  return KERR_OK;
}

static int32_t vga_text_console_set_cursor_position(struct console *con,
                                                    uint32_t x, uint32_t y) {
  if (!con)
    return -KERR_INVAL;

  struct vga_text_priv *dev_priv = container_of(con, struct vga_text_priv, con);

  if (x >= con->width || y >= con->height)
    return -KERR_RANGE;

  con->cursor_x = x;
  con->cursor_y = y;

  vga_text_update_cursor(dev_priv, con);

  return KERR_OK;
}

static int32_t vga_text_console_set_cursor_visible(struct console *con,
                                                   bool visible) {
  if (!con)
    return -KERR_INVAL;

  struct vga_text_priv *dev_priv = container_of(con, struct vga_text_priv, con);

  /*
   * Register 0x0A = Cursor Start
   * Register 0x0B = Cursor End
   */
  if (visible) {
    uint8_t value;
    /* Scanline 13-15 and enable cursor */

    hal_port_write8(dev_priv->port_base + VGA_INDEX_PORT_OFFSET, 0x0A);
    value = hal_port_read8(dev_priv->port_base + VGA_DATA_PORT_OFFSET);

    value &= ~0x1F;   // clear bits 0-4
    value |= 13;      // set cursor start scanline
    value &= ~BIT(5); // cursor disable bit

    hal_port_write8(dev_priv->port_base + VGA_DATA_PORT_OFFSET, value);

    // set cursor end scanline
    hal_port_write8(dev_priv->port_base + VGA_INDEX_PORT_OFFSET, 0x0B);
    hal_port_write8(dev_priv->port_base + VGA_DATA_PORT_OFFSET, 15);
  } else {
    /* Set the Cursor Disable bit (bit 5 of 0x0A) */
    hal_port_write8(dev_priv->port_base + VGA_INDEX_PORT_OFFSET, 0x0A);
    uint8_t value = hal_port_read8(dev_priv->port_base + VGA_DATA_PORT_OFFSET);
    value |= BIT(5);

    hal_port_write8(dev_priv->port_base + VGA_DATA_PORT_OFFSET, value);
  }

  return KERR_OK;
}

static int32_t vga_text_probe(struct platform_device *pdev) {

  struct resource *mmio_res = resource_get(
      pdev->resources, pdev->resource_count, RESOURCE_TYPE_MMIO, 0);

  struct resource *port_res = resource_get(
      pdev->resources, pdev->resource_count, RESOURCE_TYPE_PORT, 0);

  if (KERR_PTR_IS_ERR(mmio_res) || KERR_PTR_IS_ERR(port_res))
    return -KERR_INVAL;

  /*
   * Currently only VGA 80x25 text mode is supported.
   */
  if (RESOURCE_MMIO_SIZE(mmio_res) != VGA_TEXT_80X25_SIZE)
    return -KERR_NOSUP;

  struct vga_text_priv *dev_priv = kmalloc(sizeof(struct vga_text_priv));

  if (KERR_PTR_IS_ERR(dev_priv))
    return KERR_PTR_ERR(dev_priv);

  dev_priv->con.name = pdev->dev.name;
  dev_priv->con.cursor_x = 0;
  dev_priv->con.cursor_y = 0;
  dev_priv->con.width = 80;
  dev_priv->con.height = 25;
  dev_priv->buffer = (uint16_t *)PHYS_TO_VIRT(mmio_res->mmio.start);
  dev_priv->port_base = port_res->port.start;
  dev_priv->con.write = vga_text_console_write;
  dev_priv->con.clear = vga_text_console_clear;
  dev_priv->con.set_cursor_position = vga_text_console_set_cursor_position;
  dev_priv->con.set_cursor_visible = vga_text_console_set_cursor_visible;

  pdev->dev.driver_data = dev_priv;

  int32_t ret = console_register(&dev_priv->con);

  if (ret < 0) {
    kfree(dev_priv);
    return ret;
  }

  console_clear();
  console_set_cursor_visible(false);

  return KERR_OK;
}

static struct platform_driver vga_text_driver = {
    .drv =
        {
            .name = "vga-text",
        },
    .probe = vga_text_probe,
};

static int32_t __init vga_text_driver_init(void) {
  return platform_driver_register(&vga_text_driver);
};

INITCALL(INIT_DEVDRV, vga_text_driver_init);