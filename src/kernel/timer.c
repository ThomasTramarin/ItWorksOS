#include "timer.h"
#include "io.h"
#include "isr.h"
#include "klib/time.h"
#include "pic.h"

/**
 * @brief Timer driver (IRQ 0)
 */

// ticks counts the number of IRQ-0 received by the CPU, it starts from 0

void isr32_handler(struct registers *regs) {
  time_incr_ticks();
  pic_send_eoi(0);
}

void timer_init(void) {
  isr_register_handler(32, isr32_handler);

  // The chip oscillates at the frequency of 1.1931816666 MHz
  int divisor = TIMER_PIT_FREQUENCY / TIMER_FREQUENCY;

  outb(TIMER_PIT_CMD_BYTE, TIMER_PIT_CMD_PORT);
  outb(TIMER_PIT_CH_0_PORT, (uint8_t)(divisor % 0xFF));        // lobyte
  outb(TIMER_PIT_CH_0_PORT, (uint8_t)((divisor >> 8) & 0xFF)); // hibyte
}
