#include "conv.h"
#include "types.h"

void conv_itoa(int32_t value, char *str, uint8_t base) {
  char *ptr = str;

  uint32_t n;
  uint8_t digits = 0;

  bool is_negative = (base == CONV_BASE_10 && value < 0);

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

int32_t conv_atoi(const char *str, uint8_t base) {
  int32_t result = 0;
  bool is_negative = false;

  const char *ptr = str;

  // check if the number starts with the minus sign
  if (*ptr == '-') {
    is_negative = true;
    ptr++; // consume '-'
  }

  // iterate over characters
  while (*ptr != '\0') {
    uint8_t num;
    if (*ptr >= '0' && *ptr <= '9') {
      num = *ptr - '0';
    } else if (*ptr >= 'A' && *ptr <= 'F') {
      num = *ptr - 'A' + 10;
    }

    result = result * base;
    result = result + num;

    ptr++;
  }

  if (is_negative) {
    result = -result;
  }

  return result;
}