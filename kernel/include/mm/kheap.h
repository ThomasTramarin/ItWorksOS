#ifndef MM_KHEAP_H
#define MM_KHEAP_H

#include <base/stddef.h>

/**
 * @brief Initialize the kernel heap subsystem
 *
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t kheap_init(void);

/**
 * @brief Allocate memory from the kernel heap
 *
 * Allocation is intended for small and medium kernel allocations. Allocated
 * memory resides in the kernel LowMem area and is both physically and virtually
 * contiguous
 *
 * @param size Number of bytes to allocate
 * @return A pointer to the allocated memory, or a kernel error code
 *         encoded in the returned pointer
 */
void *kmalloc(size_t size);

/**
 * @brief Free a previously allocated kernel heap block
 *
 * @param ptr Pointer returned by kmalloc()
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t kfree(void *ptr);

/**
 * @brief Dump the current kernel heap state for debugging
 *
 * Prints allocation statistics and free list to the kernel syslog
 */
void kheap_dump(void);

#endif