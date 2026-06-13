#include "time.h"

static volatile uint64_t ticks = 0;

/**
 * @brief Returns the number of ticks (milliseconds) from the system start
 */
uint64_t time_get_ticks(void) { return ticks; }

// Function to increment the number of ticks by one.
void time_incr_ticks(void) { ticks++; }

/**
 * @brief Stops execution for a specified number of milliseconds
 *
 * @param ms Number of milliseconds to wait
 */
void time_sleep(uint32_t ms) {
  uint64_t start = ticks;
  while ((ticks - start) < ms) {
    asm volatile("hlt");
  }
}