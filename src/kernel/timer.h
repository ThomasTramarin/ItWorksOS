#ifndef TIMER_H
#define TIMER_H

#include "isr.h"

enum {
  TIMER_PIT_CH_0_PORT = 0x40,
  TIMER_PIT_CH_1_PORT = 0x41,
  TIMER_PIT_CH_2_PORT = 0x42,
  TIMER_PIT_CMD_PORT = 0x43,
};

/**
 * @brief PIT command byte
 *
 * Bit layout:
 *  - Bits 7-6: channel (00 = channel 0)
 *  - Bits 5-4: Access mode (11 = lobyte/hibyte)
 *  - Bits 3-1: Operating mode: Mode 3 (square wave) is 011
 *  - Bit 0: BCD mode (0 = binary)
 */
#define TIMER_PIT_CMD_BYTE 0b00110110

#define TIMER_PIT_FREQUENCY 1193182
#define TIMER_FREQUENCY 1000

void timer_init(void);

#endif