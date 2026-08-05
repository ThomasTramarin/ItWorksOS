#include "base/bit.h"
#include <arch/x86/interrupts/pic.h>
#include <arch/x86/io.h>
#include <stdint.h>

/**
 * @brief I/O ports
 */
#define X86_PIC1_COMMAND_PORT 0x20
#define X86_PIC1_DATA_PORT 0x21

#define X86_PIC2_COMMAND_PORT 0xA0
#define X86_PIC2_DATA_PORT 0xA1

/**
 * @brief ICW1 : Initialization Command Word 1 Flags
 */
#define X86_PIC_ICW1_IC4_MASK BIT(0)  /* 1 = ICW4 is required during init */
#define X86_PIC_ICW1_SNGL_MASK BIT(1) /* 1 = Single, 0 = Cascade */
#define X86_PIC_ICW1_ADI_MASK BIT(2)  /* Address interval (not used)*/
#define X86_PIC_ICW1_LTIM_MASK BIT(3) /* Trigger mode: 1 = Level, 0 = Edge */
#define X86_PIC_ICW1_INIT_MASK BIT(4) /* Mandatory bit: 1 = init mode \*/

/**
 * @brief ICW2: Initialization Command Word 2
 *
 * Sets the interrupt vector offset.
 */
#define X86_PIC_ICW2_MASTER_VECTOR_OFFSET 0x20
#define X86_PIC_ICW2_SLAVE_VECTOR_OFFSET 0x28

/**
 * @brief ICW3: Initialization Command Word 3
 */
#define X86_PIC_ICW3_MASTER BIT(2) /* The master has a slave at IRQ 2 */
#define X86_PIC_ICW3_SLAVE 2       /* Slave ID */

/**
 * @brief ICW4: Initialization Command Word 4
 */
#define X86_PIC_ICW4_8086_MASK                                                 \
  BIT(0) /* 1 = 8086/8088 mode, 0 = MCS-80/85 mode */
#define X86_PIC_ICW4_AUTO_EOI_MASK                                             \
  BIT(1) /* 1 = Automatic End of Interrupt mode */
#define X86_PIC_ICW4_BUF_SLAVE_MASK BIT(3)
#define X86_PIC_ICW4_BUF_MASTER_MASK (BIT(3) | BIT(2))
#define X86_PIC_ICW4_SFNM_MASK BIT(4) /* 1 = enabled */

/**
 * @brief OCW1: Operation Command Word 1
 * Interrupt mask (8-bit)
 *  0: mask reset (IRQ enabled)
 *  1: mask set (IRQ disabled)
 */

/**
 * @brief OCW2: Operation Command Word 2
 */
#define X86_PIC_OCW2_EOI_MASK BIT(5)

/**
 * @brief OCW3: Operation Command Word 3
 *
 */
#define X86_PIC_OCW3_READ_ISR_MASK (BIT(3) | BIT(1) | BIT(0))
#define X86_PIC_OCW3_READ_IRR_MASK (BIT(3) | BIT(1))

/**
 * @brief Introduces a short delay between legacy Port I/O operations
 *
 * The port 0x80 is not used
 */
static void x86_pic_io_wait(void) { x86_outb(0x80, 0); }

/**
 * @brief Performs PIC initialization (ICW1 - ICW4)
 *
 * - Remaps vector offsets
 * - Slave connected to master's IRQ2 line
 * - Initializes it for working with x86
 */
static void x86_pic_remap(uint8_t master_offset, uint8_t slave_offset) {

  uint8_t mask1 = x86_inb(X86_PIC1_DATA_PORT);
  uint8_t mask2 = x86_inb(X86_PIC2_DATA_PORT);

  // ICW1
  x86_outb(X86_PIC1_COMMAND_PORT,
           X86_PIC_ICW1_IC4_MASK | X86_PIC_ICW1_INIT_MASK);
  x86_pic_io_wait();
  x86_outb(X86_PIC2_COMMAND_PORT,
           X86_PIC_ICW1_IC4_MASK | X86_PIC_ICW1_INIT_MASK);
  x86_pic_io_wait();

  // ICW2
  x86_outb(X86_PIC1_DATA_PORT,
           master_offset); // set master PIC starting interrupt offset
  x86_pic_io_wait();
  x86_outb(X86_PIC2_DATA_PORT,
           slave_offset); // set slave PIC starting interrupt offset
  x86_pic_io_wait();

  // ICW3
  x86_outb(X86_PIC1_DATA_PORT, X86_PIC_ICW3_MASTER);
  x86_pic_io_wait();
  x86_outb(X86_PIC2_DATA_PORT, X86_PIC_ICW3_SLAVE);
  x86_pic_io_wait();

  // ICW4
  x86_outb(X86_PIC1_DATA_PORT, X86_PIC_ICW4_8086_MASK);
  x86_pic_io_wait();
  x86_outb(X86_PIC2_DATA_PORT, X86_PIC_ICW4_8086_MASK);
  x86_pic_io_wait();

  x86_outb(X86_PIC1_DATA_PORT, mask1);
  x86_pic_io_wait();
  x86_outb(X86_PIC2_DATA_PORT, mask2);
}

void x86_pic_unmask_irq(uint8_t irq) {

  if (irq >= 8) {
    // Enable cascade line on master PIC
    uint8_t master_mask = x86_inb(X86_PIC1_DATA_PORT);
    MASK_CLEAR(master_mask, BIT(2));
    x86_outb(X86_PIC1_DATA_PORT, master_mask);

    // Enable IRQ on slave PIC
    uint8_t slave_mask = x86_inb(X86_PIC2_DATA_PORT);
    MASK_CLEAR(slave_mask, BIT(irq - 8));
    x86_outb(X86_PIC2_DATA_PORT, slave_mask);

  } else {
    // Enable IRQ on master PIC
    uint8_t mask = x86_inb(X86_PIC1_DATA_PORT);
    MASK_CLEAR(mask, BIT(irq));
    x86_outb(X86_PIC1_DATA_PORT, mask);
  }
}

void x86_pic_mask_irq(uint8_t irq) {
  if (irq >= 8) {
    // Disable IRQ on slave PIC
    uint8_t mask = x86_inb(X86_PIC2_DATA_PORT);
    MASK_SET(mask, BIT(irq - 8));
    x86_outb(X86_PIC2_DATA_PORT, mask);

    // Check if slave has no enabled IRQs left
    if (mask == 0xFF) {
      uint8_t master_mask = x86_inb(X86_PIC1_DATA_PORT);
      MASK_SET(master_mask, BIT(2));
      x86_outb(X86_PIC1_DATA_PORT, master_mask);
    }

  } else {
    // Disable IRQ on master PIC
    uint8_t mask = x86_inb(X86_PIC1_DATA_PORT);
    MASK_SET(mask, BIT(irq));
    x86_outb(X86_PIC1_DATA_PORT, mask);
  }
}

void x86_pic_send_eoi(uint8_t irq) {

  // If the interrupt comes from the master, send EOI only to the master PIC

  // If the interrupt comes from the slave, send EOI to the slave and THEN to
  // the master PIC

  if (irq >= 8) {
    x86_outb(X86_PIC2_COMMAND_PORT, X86_PIC_OCW2_EOI_MASK);
  }

  x86_outb(X86_PIC1_COMMAND_PORT, X86_PIC_OCW2_EOI_MASK);
}

uint16_t x86_pic_read_isr(void) {
  uint16_t value = 0;

  // Master
  x86_outb(X86_PIC1_COMMAND_PORT, X86_PIC_OCW3_READ_ISR_MASK);
  value = x86_inb(X86_PIC1_COMMAND_PORT);

  // Slave
  x86_outb(X86_PIC2_COMMAND_PORT, X86_PIC_OCW3_READ_ISR_MASK);
  value |= ((uint16_t)x86_inb(X86_PIC2_COMMAND_PORT) << 8);

  return value;
}

uint16_t x86_pic_read_irr(void) {
  uint16_t value = 0;

  // Master
  x86_outb(X86_PIC1_COMMAND_PORT, X86_PIC_OCW3_READ_IRR_MASK);
  value = x86_inb(X86_PIC1_COMMAND_PORT);

  // Slave
  x86_outb(X86_PIC2_COMMAND_PORT, X86_PIC_OCW3_READ_IRR_MASK);
  value |= ((uint16_t)x86_inb(X86_PIC2_COMMAND_PORT) << 8);

  return value;
}

static void x86_pic_mask_all(void) {
  x86_outb(X86_PIC1_DATA_PORT, 0xFF);
  x86_outb(X86_PIC2_DATA_PORT, 0xFF);
}

bool x86_pic_is_spurious(uint8_t irq) {
  /*
   * Spurious interrupts may be generated by the PIC on IRQ7 (master PIC) and
   * IRQ15 (slave PIC).
   * This function checks the ISR (In-Service Register)
   */

  if (irq != 7 && irq != 15)
    return false;

  if ((x86_pic_read_isr() & BIT(irq)) != 0)
    return false;

  return true;
}

void x86_pic_handle_spurious(uint8_t irq) {
  /*
   * If the IRQ comes from the slave, send EOI only to the master
   * If the IRQ comes from the master, do not send EOI
   */
  if (irq >= 8) {
    x86_outb(X86_PIC1_COMMAND_PORT, X86_PIC_OCW2_EOI_MASK);
  }
}

void x86_pic_init(void) {
  x86_pic_remap(X86_PIC_ICW2_MASTER_VECTOR_OFFSET,
                X86_PIC_ICW2_SLAVE_VECTOR_OFFSET);

  // Disable all interrupts
  x86_pic_mask_all();
}
