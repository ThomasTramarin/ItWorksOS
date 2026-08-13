#ifndef MM_PMM_H
#define MM_PMM_H
#include <base/stddef.h>
#include <base/stdint.h>
#include <boot/boot_info.h>

enum pmm_alloc_flags {
  /* Clear all allocated frames to zero */
  PMM_ALLOC_ZERO = 1 << 0,
};

/**
 * @brief Initialize the Physical Memory Manager
 *
 * @param map_ptr Physical address of the boot memory map
 * @param count Number of entries in the memory map
 * @return KERR_OK on success, or a negative kernel error code
 */
int32_t pmm_init(const struct boot_mem_map_entry *map_ptr, uint16_t count);

/**
 * @brief Allocate physical frames
 *
 * @param min Lowest physical address allowed. Pass 0 for no lower boundary.
 * @param max Exclusive upper bound physical address. Pass 0 for no upper
 *            boundary (defaults to maximum detected physical RAM).
 * @param pages Number of frames to allocate (if > 1, frames will be contiguous)
 * @param flags Flags from enum pmm_alloc_flags
 * @param out Pysical address of the allocation
 * @return KERR_OK on success, or a negative kernel error code
 */
int32_t pmm_alloc(paddr_t min, paddr_t max, size_t pages, uint32_t flags,
                  paddr_t *out);

/**
 * @brief Free previously allocated physical frames
 *
 * @param addr Physical address of the first frame
 * @param pages Number of frames to free
 * @return KERR_OK on success, or a negative kernel error code
 */
int32_t pmm_free(paddr_t addr, size_t pages);

#endif