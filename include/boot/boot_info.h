#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <base/stdint.h>

/**
 * Bootloader magic identifier: 'IWBT' in little-endian
 * used to verify that the kernel was loaded correctly by stage2
 * and 'boot_info_t' contains valid data
 */
#define BOOT_MAGIC 0x54425749

/**
 * ACPI 3.x attributes (bitmask for boot_mem_map_entry_t.attr)
 */

/**
 * If 1: the region is enabled and valid
 * If 0: the region must be ignored
 */
#define BOOT_ATTR_ENABLED (1 << 0)

/**
 * If 1: the memory is non-volatile
 * If 0: the memory is a standard DRAM
 */
#define BOOT_ATTR_NON_VOLATILE (1 << 1)

typedef enum {
  /* Standard usable RAM. The PMM can use it */
  BOOT_TYPE_USABLE = 1,
  /* Reserved memory used by the BIOS, hardware MMIO, or system devices */
  BOOT_TYPE_RESERVED = 2,
  /* ACPI tables memory. Can be reclaimed after ACPI initialization */
  BOOT_TYPE_ACPI_RECLAIM = 3,
  /* ACPI Non-Volatile Storage. Reserved */
  BOOT_TYPE_ACPI_NVS = 4,
  /* Bad memory. Must never be accessed */
  BOOT_TYPE_BAD = 5
} boot_mem_type_t;

typedef struct __attribute__((packed)) {
  uint64_t base;
  uint64_t length;
  uint32_t type; // boot_mem_type_t
  uint32_t attr; // ACPI attributes (BOOT_ATTR_*)
} boot_mem_map_entry_t;

/**
 * The bootloader builds this struct and passes it to the kmain function
 */
typedef struct __attribute__((packed)) {
  uint16_t memory_map_count;            // number of entries
  boot_mem_map_entry_t *memory_map_ptr; // pointer to the array of entries
} boot_info_t;

#endif