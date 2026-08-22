#ifndef MM_PMM_H
#define MM_PMM_H
#include <base/stddef.h>
#include <base/stdint.h>
#include <boot/boot_info.h>
#include <mm/layout.h>

#define PMM_FRAME_SIZE 4096

enum pmm_policy { PMM_POLICY_STRICT = 0, PMM_POLICY_FALLBACK = 1 };

/* DMA16 and LOWMEM require a STRICT policy*/
#define PMM_ALLOC_DMA16(pages, out)                                            \
  pmm_alloc(0, DMA16_LIMIT, (pages), PMM_POLICY_STRICT, (out))

#define PMM_ALLOC_LOWMEM(pages, out)                                           \
  pmm_alloc(0, KERNEL_LOWMEM_LIMIT, (pages), PMM_POLICY_STRICT, (out))

/* HIGHMEM uses a FALLBACK policy (it prefers above 768 MiB) */
#define PMM_ALLOC_HIGHMEM(pages, out)                                          \
  pmm_alloc(KERNEL_LOWMEM_LIMIT, 0, (pages), PMM_POLICY_FALLBACK, (out))

#define PMM_ALLOC_ANY(pages, out)                                              \
  pmm_alloc(0, 0, (pages), PMM_POLICY_STRICT, (out))

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
 * @param out Physical address of the allocation
 * @return KERR_OK on success, or a negative kernel error code
 */
int32_t pmm_alloc(paddr_t min, paddr_t max, size_t pages,
                  enum pmm_policy policy, paddr_t *out);

/**
 * @brief Free previously allocated physical frames
 *
 * @param addr Physical address of the first frame
 * @param pages Number of frames to free
 * @return KERR_OK on success, or a negative kernel error code
 */
int32_t pmm_free(paddr_t addr, size_t pages);

#endif