#include <base/bit.h>
#include <base/sections.h>
#include <hal/port.h>
#include <irq/chip.h>
#include <irq/desc.h>
#include <irq/map.h>
#include <kernel/error.h>

/**
 * @brief I/O ports
 */
#define I8259_PIC1_COMMAND_PORT 0x20
#define I8259_PIC1_DATA_PORT 0x21

#define I8259_PIC2_COMMAND_PORT 0xA0
#define I8259_PIC2_DATA_PORT 0xA1

/**
 * @brief ICW1 : Initialization Command Word 1 Flags
 */
#define I8259_ICW1_IC4_MASK BIT(0)  /* 1 = ICW4 is required during init */
#define I8259_ICW1_SNGL_MASK BIT(1) /* 1 = Single, 0 = Cascade */
#define I8259_ICW1_ADI_MASK BIT(2)  /* Address interval (not used)*/
#define I8259_ICW1_LTIM_MASK                                                   \
  BIT(3)                            /* Trigger mode: 1 = Level, 0 = Edge       \
                                     */
#define I8259_ICW1_INIT_MASK BIT(4) /* Mandatory bit: 1 = init mode \*/

/**
 * @brief ICW2: Initialization Command Word 2
 *
 * Sets the interrupt vector offset.
 */
#define I8259_ICW2_MASTER_VECTOR_OFFSET 0x20
#define I8259_ICW2_SLAVE_VECTOR_OFFSET 0x28

/**
 * @brief ICW3: Initialization Command Word 3
 */
#define I8259_ICW3_MASTER BIT(2) /* The master has a slave at IRQ 2 */
#define I8259_ICW3_SLAVE 2       /* Slave ID */

/**
 * @brief ICW4: Initialization Command Word 4
 */
#define I8259_ICW4_8086_MASK                                                   \
  BIT(0) /* 1 = 8086/8088 mode, 0 = MCS-80/85 mode                             \
          */
#define I8259_ICW4_AUTO_EOI_MASK                                               \
  BIT(1) /* 1 = Automatic End of Interrupt mode */
#define I8259_ICW4_BUF_SLAVE_MASK BIT(3)
#define I8259_ICW4_BUF_MASTER_MASK (BIT(3) | BIT(2))
#define I8259_ICW4_SFNM_MASK BIT(4) /* 1 = enabled */

/**
 * @brief OCW1: Operation Command Word 1
 * Interrupt mask (8-bit)
 *  0: mask reset (IRQ enabled)
 *  1: mask set (IRQ disabled)
 */

/**
 * @brief OCW2: Operation Command Word 2
 */
#define I8259_OCW2_EOI_MASK BIT(5)

/**
 * @brief OCW3: Operation Command Word 3
 *
 */
#define I8259_OCW3_READ_ISR_MASK (BIT(3) | BIT(1) | BIT(0))
#define I8259_OCW3_READ_IRR_MASK (BIT(3) | BIT(1))

/**
 * @brief Introduces a short delay between legacy Port I/O operations
 *
 * The port 0x80 is not used
 */
static void __init i8259_io_wait(void) { hal_port_write8(0x80, 0); }

/**
 * @brief Performs PIC initialization (ICW1 - ICW4)
 *
 * - Remaps vector offsets
 * - Slave connected to master's IRQ2 line
 * - Initializes it for working with x86
 */
static void __init i8259_remap(uint8_t master_offset, uint8_t slave_offset) {

  uint8_t mask1 = hal_port_read8(I8259_PIC1_DATA_PORT);
  uint8_t mask2 = hal_port_read8(I8259_PIC2_DATA_PORT);

  // ICW1
  hal_port_write8(I8259_PIC1_COMMAND_PORT,
                  I8259_ICW1_IC4_MASK | I8259_ICW1_INIT_MASK);
  i8259_io_wait();
  hal_port_write8(I8259_PIC2_COMMAND_PORT,
                  I8259_ICW1_IC4_MASK | I8259_ICW1_INIT_MASK);
  i8259_io_wait();

  // ICW2
  hal_port_write8(I8259_PIC1_DATA_PORT,
                  master_offset); // set master PIC starting interrupt offset
  i8259_io_wait();
  hal_port_write8(I8259_PIC2_DATA_PORT,
                  slave_offset); // set slave PIC starting interrupt offset
  i8259_io_wait();

  // ICW3
  hal_port_write8(I8259_PIC1_DATA_PORT, I8259_ICW3_MASTER);
  i8259_io_wait();
  hal_port_write8(I8259_PIC2_DATA_PORT, I8259_ICW3_SLAVE);
  i8259_io_wait();

  // ICW4
  hal_port_write8(I8259_PIC1_DATA_PORT, I8259_ICW4_8086_MASK);
  i8259_io_wait();
  hal_port_write8(I8259_PIC2_DATA_PORT, I8259_ICW4_8086_MASK);
  i8259_io_wait();

  hal_port_write8(I8259_PIC1_DATA_PORT, mask1);
  i8259_io_wait();
  hal_port_write8(I8259_PIC2_DATA_PORT, mask2);
}

static void i8259_mask_irq(struct irq_desc *desc) {
  if (desc->hwirq >= 8) {
    // Disable IRQ on slave PIC
    uint8_t mask = hal_port_read8(I8259_PIC2_DATA_PORT);
    MASK_SET(mask, BIT(desc->hwirq - 8));
    hal_port_write8(I8259_PIC2_DATA_PORT, mask);

    // Check if slave has no enabled IRQs left
    if (mask == 0xFF) {
      uint8_t master_mask = hal_port_read8(I8259_PIC1_DATA_PORT);
      MASK_SET(master_mask, BIT(2));
      hal_port_write8(I8259_PIC1_DATA_PORT, master_mask);
    }

  } else {
    // Disable IRQ on master PIC
    uint8_t mask = hal_port_read8(I8259_PIC1_DATA_PORT);
    MASK_SET(mask, BIT(desc->hwirq));
    hal_port_write8(I8259_PIC1_DATA_PORT, mask);
  }
}

static void __init i8259_mask_all(void) {
  hal_port_write8(I8259_PIC1_DATA_PORT, 0xFF);
  hal_port_write8(I8259_PIC2_DATA_PORT, 0xFF);
}

static void i8259_unmask_irq(struct irq_desc *desc) {

  if (desc->hwirq >= 8) {
    // Enable cascade line on master PIC
    uint8_t master_mask = hal_port_read8(I8259_PIC1_DATA_PORT);
    MASK_CLEAR(master_mask, BIT(2));
    hal_port_write8(I8259_PIC1_DATA_PORT, master_mask);

    // Enable IRQ on slave PIC
    uint8_t slave_mask = hal_port_read8(I8259_PIC2_DATA_PORT);
    MASK_CLEAR(slave_mask, BIT(desc->hwirq - 8));
    hal_port_write8(I8259_PIC2_DATA_PORT, slave_mask);

  } else {
    // Enable IRQ on master PIC
    uint8_t mask = hal_port_read8(I8259_PIC1_DATA_PORT);
    MASK_CLEAR(mask, BIT(desc->hwirq));
    hal_port_write8(I8259_PIC1_DATA_PORT, mask);
  }
}

static void i8259_send_eoi(struct irq_desc *desc) {

  // If the interrupt comes from the master, send EOI only to the master PIC

  // If the interrupt comes from the slave, send EOI to the slave and THEN to
  // the master PIC

  if (desc->hwirq >= 8) {
    hal_port_write8(I8259_PIC2_COMMAND_PORT, I8259_OCW2_EOI_MASK);
  }

  hal_port_write8(I8259_PIC1_COMMAND_PORT, I8259_OCW2_EOI_MASK);
}

static uint16_t i8259_read_isr(void) {
  uint16_t value = 0;

  // Master
  hal_port_write8(I8259_PIC1_COMMAND_PORT, I8259_OCW3_READ_ISR_MASK);
  value = hal_port_read8(I8259_PIC1_COMMAND_PORT);

  // Slave
  hal_port_write8(I8259_PIC2_COMMAND_PORT, I8259_OCW3_READ_ISR_MASK);
  value |= ((uint16_t)hal_port_read8(I8259_PIC2_COMMAND_PORT) << 8);

  return value;
}

static uint16_t i8259_read_irr(void) {
  uint16_t value = 0;

  // Master
  hal_port_write8(I8259_PIC1_COMMAND_PORT, I8259_OCW3_READ_IRR_MASK);
  value = hal_port_read8(I8259_PIC1_COMMAND_PORT);

  // Slave
  hal_port_write8(I8259_PIC2_COMMAND_PORT, I8259_OCW3_READ_IRR_MASK);
  value |= ((uint16_t)hal_port_read8(I8259_PIC2_COMMAND_PORT) << 8);

  return value;
}

static bool i8259_is_spurious(uint32_t hwirq) {
  /*
   * Spurious interrupts may be generated by the PIC on IRQ7 (master PIC) and
   * IRQ15 (slave PIC).
   * This function checks the ISR (In-Service Register)
   */

  if (hwirq != 7 && hwirq != 15)
    return false;

  if ((i8259_read_isr() & BIT(hwirq)) != 0)
    return false;

  return true;
}

static int32_t i8259_handle_vector(uint32_t vector, hwirq_t *hwirq) {

  if (!hwirq)
    return -KERR_INVAL;

  if (vector >= I8259_ICW2_MASTER_VECTOR_OFFSET &&
      vector < I8259_ICW2_MASTER_VECTOR_OFFSET + 8) {

    *hwirq = vector - I8259_ICW2_MASTER_VECTOR_OFFSET;

  } else if (vector >= I8259_ICW2_SLAVE_VECTOR_OFFSET &&
             vector < I8259_ICW2_SLAVE_VECTOR_OFFSET + 8) {

    *hwirq = 8 + (vector - I8259_ICW2_SLAVE_VECTOR_OFFSET);

  } else {
    return -KERR_NOENT;
  }

  if (i8259_is_spurious(*hwirq)) {
    /*
     * If the IRQ comes from the slave, send EOI only to the master
     * If the IRQ comes from the master, do not send EOI
     */
    if (*hwirq == 15) {
      hal_port_write8(I8259_PIC1_COMMAND_PORT, I8259_OCW2_EOI_MASK);
    }

    return -KERR_NOENT;
  }

  return KERR_OK;
}

static struct irq_chip i8259_chip = {
    .name = "i8259-pic-chip",
    .handle_vector = i8259_handle_vector,
    .disable = i8259_mask_irq,
    .enable = i8259_unmask_irq,
    .eoi = i8259_send_eoi,
};
static struct irq_map i8259_map;

struct irq_chip *i8259_get_chip(void) { return &i8259_chip; }
struct irq_map *i8259_get_map(void) { return &i8259_map; }

int32_t __init i8259_init(void) {
  i8259_remap(I8259_ICW2_MASTER_VECTOR_OFFSET, I8259_ICW2_SLAVE_VECTOR_OFFSET);

  // Disable all interrupts
  i8259_mask_all();

  // Register a map for the controller
  KERR_TRY(irq_map_init(&i8259_map, "i8259-pic-map", 0, 16));

  // Configure IRQ descriptors
  for (hwirq_t hwirq = 0; hwirq < i8259_map.count; hwirq++) {
    irq_t irq = irq_map_hwirq_to_irq(&i8259_map, hwirq);

    KERR_TRY(irq_desc_configure(irq, hwirq, &i8259_map, &i8259_chip));
  }

  return KERR_OK;
}