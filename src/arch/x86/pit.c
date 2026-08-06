// #include <arch/x86/interrupts/pic.h>
// #include <arch/x86/io.h>
// #include <arch/x86/pit.h>
// #include <klib/time.h>

/**
 * @brief Timer driver (IRQ 0)
 */

// ticks counts the number of IRQ-0 received by the CPU, it starts from 0

// static void x86_isr32_handler(struct x86_registers *regs) {
//   time_incr_ticks();
//   x86_pic_send_eoi(0);
// }

// void x86_timer_init(void) {

//   // The chip oscillates at the frequency of 1.1931816666 MHz
//   int divisor = TIMER_PIT_FREQUENCY / TIMER_FREQUENCY;

//   x86_outb(TIMER_PIT_CMD_BYTE, TIMER_PIT_CMD_PORT);
//   x86_outb(TIMER_PIT_CH_0_PORT, (uint8_t)(divisor % 0xFF));        // lobyte
//   x86_outb(TIMER_PIT_CH_0_PORT, (uint8_t)((divisor >> 8) & 0xFF)); // hibyte
// }
