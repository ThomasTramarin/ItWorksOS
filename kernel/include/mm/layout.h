#ifndef MM_LAYOUT_H
#define MM_LAYOUT_H

#include <base/stdint.h>

/**
 * @file layout.h
 *
 * @brief Physical and virtual memory layout of the kernel
 */

/* Higher Half Kernel offset (3 GiB) */
#define KERNEL_VIRT_OFFSET 0xC0000000ULL

#define DMA16_LIMIT (16ULL * 1024ULL * 1024ULL)

/* Size of the physical RAM directly mapped in LowMem (768 MiB) */
#define KERNEL_LOWMEM_LIMIT (768ULL * 1024ULL * 1024ULL)

/* First virtual address after the kernel LowMem mapping */
#define KERNEL_LOWMEM_END (KERNEL_VIRT_OFFSET + KERNEL_LOWMEM_LIMIT)

/**
 * @brief Converts a physical address (LowMem region) into a virtual kernel
 * address
 * @note This is only valid for physical memory below 768 MiB
 */
#define PHYS_TO_VIRT(phys) ((vaddr_t)((uint64_t)(phys) + KERNEL_VIRT_OFFSET))

/**
 * @brief Converts an higher-half kernel virtual address into a physical LowMem
 * address
 * @note This is only valid for virtual addresses >= 0xC0000000 and < 0xF0000000
 */
#define VIRT_TO_PHYS(phys) ((vaddr_t)((uint64_t)(phys) + KERNEL_VIRT_OFFSET))

#endif