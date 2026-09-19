#ifndef ARCH_X86_MM_H
#define ARCH_X86_MM_H

#include <base/stddef.h>

struct arch_vm_space {
  /**
   * @brief Page Directory phisical address
   *
   */
  paddr_t pd_phys;
};

/**
 * @brief Return the virtual memory page size
 *
 * @return Page size in bytes
 */
size_t arch_vm_page_size(void);

/**
 * @brief Initialize an architecture-specific virtual memory space
 *
 * @param vm Virtual memory space to initialize
 * @return KERR_OK on success, or a negative error code
 */
int32_t arch_vm_space_init(struct arch_vm_space *vm);

/**
 * @brief Destroy an architecture-specific virtual memory space
 *
 * @param vm Virtual memory space to destroy
 * @return KERR_OK on success, or a negative error code
 */
int32_t arch_vm_space_destroy(struct arch_vm_space *vm);

/**
 * @brief Attach shared kernel mappings to a virtual memory space
 *
 * Copies the kernel PDE entries into the virtual memory space
 * so that kernel page tables are shared between address spaces
 *
 * @param vm Virtual memory space of the process
 * @param kernel_vm Virtual memory space of the kernel
 * @return KERR_OK on success, or a negative error code
 */
int32_t arch_vm_space_attach_kernel(struct arch_vm_space *vm,
                                    const struct arch_vm_space *kernel_vm);

/**
 * @brief Map physical memory into a virtual memory space
 *
 * @param vm Virtual memory space
 * @param virt Starting virtual address
 * @param phys Starting physical address
 * @param size Size of the mapping in bytes
 * @param flags Mapping flags
 * @return KERR_OK on success, or a negative error code
 */
int32_t arch_vm_map(struct arch_vm_space *vm, vaddr_t virt, paddr_t phys,
                    size_t size, uint32_t flags);

/**
 * @brief Unmap a virtual memory range
 *
 * @param vm Virtual memory space
 * @param virt Starting virtual address
 * @param size Size of the range in bytes
 * @return KERR_OK on success, or a negative error code
 */
int32_t arch_vm_unmap(struct arch_vm_space *vm, vaddr_t virt, size_t size);

/**
 * @brief Change the permissions of a virtual memory range
 *
 * @param vm Virtual memory space
 * @param virt Starting virtual address
 * @param size Size of the range in bytes
 * @param flags New mapping flags
 * @return KERR_OK on success, or a negative error code
 */
int32_t arch_vm_protect(struct arch_vm_space *vm, vaddr_t virt, size_t size,
                        uint32_t flags);

/**
 * @brief Load a virtual memory space into the CPU
 *
 * @param vm Virtual memory space to load
 * @return KERR_OK on success, or a negative kernel error code
 */
int32_t arch_vm_space_load(struct arch_vm_space *vm);

#endif