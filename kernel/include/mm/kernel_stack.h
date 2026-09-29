#ifndef MM_KERNEL_STACK_H
#define MM_KERNEL_STACK_H

#include <base/stdint.h>

/**
 * Kernel stacks are allocated from a dedicated virtual address region outside
 * the kernel direct map. The current stack pointer is not stored in struct
 * kernel_stack. It is part of the architecture-specific CPU context.
 */

/* Number of pages allocated to each kernel stack */
#define KERNEL_STACK_PAGES 4

#define KERNEL_STACK_BASE 0xF0000000

/**
 * @brief Describes the virtual address range of a process's kernel stack
 *
 * The stack pointer is initialized according to the stack growth direction
 * defined by the target architecture
 */
struct kernel_stack {
  vaddr_t base;
  vaddr_t size;
};

/**
 *
 * @param stack Kernel stack descriptor to initialize
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t kstack_alloc(struct kernel_stack *stack);

/**
 * Unmaps the stack from the kernel address space and releases physical memory
 *
 * @param stack Kernel stack to free
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t kstack_free(struct kernel_stack *stack);

#endif