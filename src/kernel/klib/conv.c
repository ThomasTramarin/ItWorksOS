#include "conv.h"
#include "types.h"

/**
 * @brief Converts an integer to its ASCII representation
 * @param value The integer
 * @param str Destination buffer. Must be at least CONV_ITOA_MAX_CHARS_BASE_X to
 * prevent buffer overflows
 * @param base  Numeric base (e.g., CONV_ITOA_BASE_10, CONV_ITOA_BASE_16).
 *
 * @note If base is 10 and value is negative, a '-' sign is added. All other
 * bases treat the value as raw bit patterns (unsigned).
 */
void conv_itoa(int32_t value, char *str, uint8_t base) {
  char *ptr = str;

  uint32_t n;
  uint8_t digits = 0;

  bool is_negative = (base == CONV_ITOA_BASE_10 && value < 0);

  if (is_negative) {
    *ptr++ = '-';
    n = (uint32_t)(-value);
  } else {
    n = (uint32_t)value;
  }

  if (n == 0) {
    *ptr++ = '0';
    *ptr = '\0';
    return;
  }

  // Extracts digits. The logic works well for any base up to 16
  while (n != 0) {
    uint8_t remainder = n % base;
    *ptr++ = (remainder > 9) ? (remainder - 10) + 'A' : remainder + '0';
    digits++;
    n /= base;
  }

  *ptr = '\0';

  // reverse string: only the numeric part is swapped, preserving the '-' sign.
  char *start = is_negative ? (str + 1) : str;

  for (int i = 0; i < digits / 2; i++) {
    char tmp = start[i];
    start[i] = start[digits - 1 - i];
    start[digits - 1 - i] = tmp;
  }
}