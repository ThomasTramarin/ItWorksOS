#ifndef KERNEL_BOOT_H
#define KERNEL_BOOT_H

#include <boot/boot_info.h>

/**
 * @brief Boot state (information) maintained by the kernel
 * This struct contains the same information boot_info exposes, but it's
 * supposed to be used by the kernel, so for example contains virtual addresses,
 * instead of physical addresses.
 */
struct boot_state {
  uint16_t memory_map_count;
  struct boot_mem_map_entry *memory_map;
  struct boot_video video;
};

/**
 * @brief Take the ownership of the boot_info struct provided by Stage2
 *        bootloader and build struct boot_state (used by the kernel)
 *
 * The kernel does not modify the stage2 memory.
 *
 * After calling this function, the physical memory manager can mark Stage2
 * memory as usabe.
 *
 * @param info Stage2 boot_info pointer
 */
void boot_init(struct boot_info *info);

/**
 * @brief Get the 'struct boot_state' pointer (stored in the kernel memory)
 *
 * Since the kernel saves boot_state in its static memory, this information is
 * always valid, after calling boot_init()
 *
 * @return Pointer to the kernel-owned boot_state struct
 */
struct boot_state *boot_get_state(void);

#endif