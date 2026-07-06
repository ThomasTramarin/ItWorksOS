#ifndef TIME_H
#define TIME_H

#include <base/stdint.h>

uint64_t time_get_ticks(void);
void time_incr_ticks(void);

void time_sleep(uint32_t ms);

#endif