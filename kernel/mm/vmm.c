#include <base/stddef.h>
#include <hal/mm.h>
#include <kernel/error.h>
#include <mm/mm.h>
#include <mm/vmm.h>

/**
 * @brief Virtual Memory Area
 *
 */
struct vma {
  vaddr_t start;
  vaddr_t end;
  uint32_t flags;
};

/**
 * @brief Virtual Memory Space
 *
 */
struct vm_space {
  struct hal_vm_space *arch;
};

/**
 * @brief Kernel Virtual Memory Space
 * The kernel is mapped into memory starting from 0xC0000000
 * (higher-half kernel).
 *
 * The first 768 MiB of kernel virtual space are directly mapped to the first
 * physical 768 MiB.
 *
 * Kernel virtual memory space is statically allocated because kmalloc still
 * does not exists
 */
static struct vm_space kernel_vm_space;
extern struct hal_vm_space kernel_arch_vm_space;

int32_t vmm_init(void) {

  kernel_vm_space.arch = &kernel_arch_vm_space;

  /* Try to create a virtual kernel space */
  KERR_TRY(hal_mm_space_init(kernel_vm_space.arch, NULL));

  return KERR_OK;
};
