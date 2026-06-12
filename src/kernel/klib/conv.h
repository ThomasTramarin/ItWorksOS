#ifndef CONV_H
#define CONV_H

#include "types.h"

#define CONV_ITOA_BASE_2 2
#define CONV_ITOA_BASE_8 8
#define CONV_ITOA_BASE_10 10
#define CONV_ITOA_BASE_16 16

#define CONV_ITOA_MAX_CHARS_BASE_2 33
#define CONV_ITOA_MAX_CHARS_BASE_8 12
#define CONV_ITOA_MAX_CHARS_BASE_10 12
#define CONV_ITOA_MAX_CHARS_BASE_16 9

void conv_itoa(int32_t value, char *str, uint8_t base);

#endif