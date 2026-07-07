#include <arch/x86/cpu/io.h>
#include <drivers/video/vga/vga.h>

static volatile struct vga_cell *text_buffer = (struct vga_cell *)VGA_MEM_ADDR;
static struct vga_cell back_buffer[VGA_ROWS * VGA_COLS];
static struct vga_dirty_rect dirty;

void vga_init(void) {

  // No dirty region at start
  dirty.x1 = VGA_COLS;
  dirty.y1 = VGA_ROWS;
  dirty.x2 = 0;
  dirty.y2 = 0;

  vga_cursor_set_shape(13, 15); // cursor style -> _
  vga_cursor_set_xy(0, 0);      // reset the cursor at (0,0)

  vga_cursor_disable(); // disable the cursor (it will be enabled on user input)
}

static uint16_t vga_xy_to_pos(uint8_t x, uint8_t y) {
  return (y * VGA_COLS) + x;
}

static void vga_pos_to_xy(uint16_t pos, uint8_t *x, uint8_t *y) {
  if (y)
    *y = pos / VGA_COLS;
  if (x)
    *x = pos % VGA_COLS;
}

void vga_set_cell(uint8_t x, uint8_t y, struct vga_cell cell) {

  // check bounds
  if (x >= VGA_COLS || y >= VGA_ROWS)
    return;

  // update backbuffer
  back_buffer[vga_xy_to_pos(x, y)] = cell;

  // update dirty rect
  if (x < dirty.x1)
    dirty.x1 = x;
  if (y < dirty.y1)
    dirty.y1 = y;

  if (x > dirty.x2)
    dirty.x2 = x;
  if (y > dirty.y2)
    dirty.y2 = y;
}

bool vga_get_cell(uint8_t x, uint8_t y, struct vga_cell *out) {

  if (out == NULL)
    return false;

  if (x >= VGA_COLS || y >= VGA_ROWS)
    return false;

  *out = back_buffer[vga_xy_to_pos(x, y)];

  return true;
}

void vga_fill_rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                   struct vga_cell cell) {

  // normalize coords
  if (x1 > x2) {
    uint8_t tmp = x1;
    x1 = x2;
    x2 = tmp;
  }

  if (y1 > y2) {
    uint8_t tmp = y1;
    y1 = y2;
    y2 = tmp;
  }

  // fill rectangle
  for (uint8_t y = y1; y <= y2; y++) {
    if (y >= VGA_ROWS)
      break;

    for (uint8_t x = x1; x <= x2; x++) {
      if (x >= VGA_COLS)
        break;

      back_buffer[vga_xy_to_pos(x, y)] = cell;
    }
  }

  // mark dirty
  if (x1 < dirty.x1)
    dirty.x1 = x1;
  if (y1 < dirty.y1)
    dirty.y1 = y1;
  if (x2 > dirty.x2)
    dirty.x2 = x2;
  if (y2 > dirty.y2)
    dirty.y2 = y2;
}

void vga_flush(void) {

  // if backbuffer is still up-to-date
  if (dirty.x1 > dirty.x2 || dirty.y1 > dirty.y2)
    return;

  // copy dirty portion from backbuffer to VGA buffer
  for (uint8_t y = dirty.y1; y <= dirty.y2; y++) {
    for (uint8_t x = dirty.x1; x <= dirty.x2; x++) {
      text_buffer[vga_xy_to_pos(x, y)] = back_buffer[vga_xy_to_pos(x, y)];
    }
  }

  // Invalidate dirty rect (now backbuffer is synced with the real VGA buffer)
  dirty.x1 = VGA_COLS;
  dirty.y1 = VGA_ROWS;
  dirty.x2 = 0;
  dirty.y2 = 0;
}

void vga_scroll(struct vga_cell_attr blank_attr) {
  // shift all rows up by one
  for (uint8_t y = 1; y < VGA_ROWS; y++) {
    for (uint8_t x = 0; x < VGA_COLS; x++) {
      back_buffer[vga_xy_to_pos(x, y - 1)] = back_buffer[vga_xy_to_pos(x, y)];
    }
  }

  // make the last row blank
  struct vga_cell blank = {
      .ch = ' ',
      .attr = blank_attr,
  };

  for (int i = 0; i < VGA_COLS; i++) {
    back_buffer[vga_xy_to_pos(i, VGA_ROWS - 1)] = blank;
  }

  // make full screen dirty (the next flush will copy the entire backbuffer into
  // VGA memory)
  dirty.x1 = 0;
  dirty.y1 = 0;
  dirty.x2 = VGA_COLS - 1;
  dirty.y2 = VGA_ROWS - 1;
}

void vga_cursor_enable(void) {
  // disable Cursor Disable flag (CD = 0)
  outb(VGA_INDEX_PORT, VGA_CURSOR_START_REG);
  uint8_t start = inb(VGA_DATA_PORT);
  start &= ~0x20;

  outb(VGA_INDEX_PORT, VGA_CURSOR_START_REG);
  outb(VGA_DATA_PORT, start);
}

void vga_cursor_disable(void) {
  // set the CD (Cursor Disable) bit to 1 (bit n 5)
  outb(VGA_INDEX_PORT, VGA_CURSOR_START_REG);

  uint8_t start = inb(VGA_DATA_PORT);
  start |= 0x20;

  outb(VGA_INDEX_PORT, VGA_CURSOR_START_REG);
  outb(VGA_DATA_PORT, start);
}

void vga_cursor_set_xy(uint8_t x, uint8_t y) {
  if (x >= VGA_COLS || y >= VGA_ROWS)
    return;

  //
  uint16_t pos = vga_xy_to_pos(x, y);

  outb(VGA_INDEX_PORT, VGA_CURSOR_LOBYTE_POS_REG);
  outb(VGA_DATA_PORT, (pos & 0xFF));

  outb(VGA_INDEX_PORT, VGA_CURSOR_HIBYTE_POS_REG);
  outb(VGA_DATA_PORT, (pos >> 8) & 0xFF);
}

void vga_cursor_get_xy(uint8_t *x, uint8_t *y) {
  uint16_t pos = 0;
  outb(VGA_INDEX_PORT, VGA_CURSOR_LOBYTE_POS_REG);
  pos = inb(VGA_DATA_PORT);

  outb(VGA_INDEX_PORT, VGA_CURSOR_HIBYTE_POS_REG);
  pos = pos | (inb(VGA_DATA_PORT) << 8);

  vga_pos_to_xy(pos, x, y);
}

void vga_cursor_set_shape(uint8_t start_scanline, uint8_t end_scanline) {
  if (start_scanline > end_scanline)
    return;

  if (start_scanline > 15 || end_scanline > 15)
    return;

  outb(VGA_INDEX_PORT, VGA_CURSOR_START_REG);
  outb(VGA_DATA_PORT, start_scanline);

  outb(VGA_INDEX_PORT, VGA_CURSOR_END_REG);
  outb(VGA_DATA_PORT, end_scanline);
}