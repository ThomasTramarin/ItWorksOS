#include "pic.h"
#include "io.h"

typedef enum {
  PIC1_COMMAND_PORT = 0x20,
  PIC1_DATA_PORT = 0x21,
  PIC2_COMMAND_PORT = 0xA0,
  PIC2_DATA_PORT = 0xA1
} PIC_PORT;

// ICW1: Initialization Commands
typedef enum {
  PIC_ICW1_IC4 = 0x01,  // PIC expects to receive ICW4 during initialization
  PIC_ICW1_INIT = 0x10, // Initialize PIC
} PIC_ICW1;

typedef enum {
  PIC_ICW4_8086 = 0x01,
  PIC_ICW4_AUTO_EOI = 0x02,
  PIC_ICW4_BUFFER_MASTER = 0x4,
  PIC_ICW4_BUFFER_SLAVE = 0x0,
  PIC_ICW4_BUFFERED = 0x8,
  PIC_ICW4_SPECIAL_FULLY_NESTED_MODE = 0x10
} PIC_ICW4;

void pic_init(void) {
  // ICW1
  outb(PIC1_COMMAND_PORT, PIC_ICW1_IC4 | PIC_ICW1_INIT);
  io_wait();
  outb(PIC2_COMMAND_PORT, PIC_ICW1_IC4 | PIC_ICW1_INIT);
  io_wait();

  // ICW2
  outb(PIC1_DATA_PORT, 0x20); // The master PIC starts now from interrupt 32
  io_wait();
  outb(PIC2_DATA_PORT, 0x28); // The slave PIC starts now from interrupt 40
  io_wait();

  // ICW3
  outb(PIC1_DATA_PORT, 0x04); // Master knows that it has a slave at IRQ2
  io_wait();
  outb(PIC2_DATA_PORT, 0x02); // Set slave ID = 2
  io_wait();

  // ICW4
  outb(PIC1_DATA_PORT, PIC_ICW4_8086);
  io_wait();
  outb(PIC2_DATA_PORT, PIC_ICW4_8086);
  io_wait();

  pic_set_all_irqs(false); // Disable all irqs
  pic_enable_irq(0);       // Enable timer
}

void pic_enable_irq(uint8_t irq) {

  if (irq < 8) {
    // Master
    uint8_t mask = inb(PIC1_DATA_PORT);
    mask &= ~(1 << irq);
    outb(PIC1_DATA_PORT, mask);
  } else {
    // Slave
    uint8_t mask = inb(PIC2_DATA_PORT);
    mask &= ~(1 << (irq - 8));
    outb(PIC2_DATA_PORT, mask);
  }
  io_wait();
}

void pic_disable_irq(uint8_t irq) {

  if (irq < 8) {
    // Master
    uint8_t mask = inb(PIC1_DATA_PORT);
    mask |= (1 << irq);
    outb(PIC1_DATA_PORT, mask);
  } else {
    // Slave
    uint8_t mask = inb(PIC2_DATA_PORT);
    mask |= (1 << (irq - 8));
    outb(PIC2_DATA_PORT, mask);
  }
  io_wait();
}

/**
 * @param val true = enable all irqs (master and slave), false = disable all
 * irqs (master and slave)
 */
void pic_set_all_irqs(bool enable) {
  uint8_t mask = enable ? 0x00 : 0xFF;

  outb(PIC1_DATA_PORT, mask);
  io_wait();
  outb(PIC2_DATA_PORT, mask);
  io_wait();
}

void pic_send_eoi(uint8_t irq) {

  // If the interrupt comes from the master, send EOI only to the maser PIC

  // If the interrupt comes from the slave, send EOI to the slave and THEN to
  // the maser PIC

  if (irq >= 8) {
    outb(PIC2_COMMAND_PORT, 0x20);
    io_wait();
  }

  outb(PIC1_COMMAND_PORT, 0x20);
  io_wait();
}