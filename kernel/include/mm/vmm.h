#ifndef MM_VMM_H
#define MM_VMM_H

#include <arch/mm.h>
#include <base/bit.h>
#include <base/stddef.h>
#include <klib/list.h>

#define VM_READ BIT(0)
#define VM_WRITE BIT(1)
#define VM_EXEC BIT(2)
#define VM_USER BIT(3)
#define VM_GROWSDOWN BIT(4)

/**
 * @brief Virtual Memory Area
 *
 * Represents a contiguous virtual memory region with specific permissions
 */
struct vma {
  vaddr_t start;
  vaddr_t end;
  uint32_t flags;
  struct list_node vmas_node;
};

/**
 * @brief Virtual Memory Space
 *
 * Represents a process virtual address space
 */
struct vm_space {
  struct list vmas;

  /**
   * @brief Architecture-specific virtual memory data
   */
  struct arch_vm_space arch;
};

/**
 * @brief Initialize the virtual memory manager, including the kernel virtual
 * memory mappings
 *
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t vmm_init(void);

/**
 * @brief Initialize a process virtual address space
 *
 * Creates the architecture-specific page table and initializes the user-space
 * VMA list
 *
 * @param vm Virtual address space to initialize
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t vm_space_init(struct vm_space *vm);

/**
 * @brief Destroy a process virtual address space
 *
 * Releases the resources owned by the virtual address space
 *
 * @param vm Virtual address space to destroy
 */
int32_t vm_space_destroy(struct vm_space *vm);

/**
 * @brief Find the VMA containing a virtual address
 *
 * @param vm Virtual address space
 * @param virt Virtual address to look up
 *
 * @return Pointer to the containing VMA, or NULL if none exists
 */
struct vma *vm_find_vma(struct vm_space *vm, vaddr_t virt);

/**
 * @brief Create a virtual to physical mapping
 *
 * Maps the range [virt, virt + size) to [phys, phys + size)
 *
 * Creates a VMA for the specified range and establishes
 * the corresponding architecture-specific page table mappings
 *
 * @param vm Virtual address space
 * @param virt Start virtual address
 * @param phys Start physical address
 * @param size Size of the mapping in bytes
 * @param flags Mapping permissions (VM_*)
 *
 * @return KERR_OK on success, or a negative kernel error code
 */
int32_t vm_map(struct vm_space *vm, vaddr_t virt, paddr_t phys, size_t size,
               uint32_t flags);

/**
 * @brief Remove a virtual to physical mapping
 *
 * Removes the architecture-specific page table mappings and
 * the corresponding VMA for the specified range
 *
 * @param vm Virtual address space
 * @param virt Start virtual address
 * @param size Size of the mapping in bytes
 *
 * @return KERR_OK on success, or a negative kernel error code
 */
int32_t vm_unmap(struct vm_space *vm, vaddr_t virt, size_t size);

/**
 * @brief Change the permissions of an existing mapping
 *
 * Updates both the VMA permissions and the corresponding
 * architecture-specific page table mappings
 *
 * @param vm Virtual address space
 * @param virt Start virtual address
 * @param size Size of the mapping in bytes
 * @param flags New mapping permissions (VM_*)
 *
 * @return KERR_OK on success, or a negative kernel error code
 */
int32_t vm_protect(struct vm_space *vm, vaddr_t virt, size_t size,
                   uint32_t flags);

/**
 * @brief Load the specified virtual memory space into the CPU
 *
 * @param vm Virtual address space
 * @return KERR_OK on success, or a negative kernel error code
 */
int32_t vm_space_load(struct vm_space *vm);

#endif