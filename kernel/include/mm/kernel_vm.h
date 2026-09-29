#ifndef MM_KERNEL_VM_H
#define MM_KERNEL_VM_H

#include <base/stddef.h>

struct arch_vm_space;

/**
 * @brief Manage kernel virtual memory mappings
 *
 * The kernel virtual memory space range is [3GiB, 4GiB), so these
 * function must use virtual addresses within this range.
 */

/**
 * @brief Attach shared kernel mappings to a virtual memory space
 *
 * @param vm Virtual memory space of the process
 * @return KERR_OK on success, or a negative error code
 */
int32_t vm_kern_attach(struct arch_vm_space *vm);

int32_t vm_kern_map(vaddr_t virt, paddr_t phys, size_t size, uint32_t flags);

int32_t vm_kern_unmap(vaddr_t virt, size_t size);

int32_t vm_kern_protect(vaddr_t virt, size_t size, uint32_t flags);

int32_t vm_kern_translate(vaddr_t virt, paddr_t *phys);

/**
 * @brief Initialize the kernel virtual memory
 *
 * Creates the kernel page table hierarchy and the
 * initial kernel virtual memory layout
 *
 * @return KERR_OK on success, or a negative kernel error code
 */
int32_t vm_kern_init(void);

#endif