#ifndef MM_MM_H
#define MM_MM_H

/* Include memory subsystems */
#include <mm/kheap.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

/**
 * @brief Initialize the entire memory management subsystem
 *
 * Initialize in sequence:
 *  1. Physical Memory Manager
 *  2. Virtual Memory Manager
 *  3. Kernel Heap
 *
 * @return KERR_OK or negative kernel error code
 */
int32_t mm_init(void);

#endif