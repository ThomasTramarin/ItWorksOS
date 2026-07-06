#ifndef BOOT_INFO
#define BOOT_INFO

#include <base/stdint.h>

/**
 * The bootloader builds this struct and passes it to the kmain function
 */
typedef struct __attribute__((packed)) {
  uint8_t magic[4];
  uint8_t boot_drive;
} boot_info_t;

#endif