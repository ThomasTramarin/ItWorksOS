#include <arch/cpu/control.h>
#include <base/bit.h>
#include <base/stdint.h>
#include <hal/mm.h>
#include <kernel/error.h>
#include <klib/memory.h>
#include <mm/pmm.h>

#define X86_PAGE_SIZE 4096
#define X86_PDE_COUNT 1024
#define X86_PTE_COUNT 1024

#define X86_KERNEL_PDE_BASE 768

/* Page Directory Entry */
#define X86_PDE_PRESENT BIT(0)
#define X86_PDE_WRITABLE BIT(1)
#define X86_PDE_USER BIT(2)
#define X86_PDE_PWT BIT(3)
#define X86_PDE_PCD BIT(4)
#define X86_PDE_ACCESSED BIT(5)
#define X86_PDE_PAGE_SIZE BIT(7)

/* Page Table Entry */
#define X86_PTE_PRESENT BIT(0)
#define X86_PTE_WRITABLE BIT(1)
#define X86_PTE_USER BIT(2)
#define X86_PTE_PWT BIT(3)
#define X86_PTE_PCD BIT(4)
#define X86_PTE_ACCESSED BIT(5)
#define X86_PTE_DIRTY BIT(6)
#define X86_PTE_PAT BIT(7)
#define X86_PTE_GLOBAL BIT(8)

/* x86 specific */
struct hal_vm_space {
  paddr_t page_directory;
};

struct hal_vm_space kernel_arch_vm_space;

int32_t hal_mm_space_init(struct hal_vm_space *space_out,
                          struct hal_vm_space *kernel_space) {

  if (!space_out)
    return -KERR_INVAL;

  paddr_t pd_phys; // page directory physical address

  // get one page from the PMM for storing PDE
  KERR_TRY(PMM_ALLOC_LOWMEM(1, &pd_phys));

  space_out->page_directory = pd_phys;

  /*
   * Clear the entire page directory
   */
  uint32_t *pd = (uint32_t *)PHYS_TO_VIRT(pd_phys);
  memset(pd, 0, X86_PAGE_SIZE);

  if (kernel_space == NULL) {

    /*
     * PSE has been enabled by the bootloader
     *
     * The kernel has the virtual region 0xC0000000 - 0xEFFFFFFF (768 MiB)
     * directly mapped to physical memory 0x00000000 - 0x30000000
     *
     * For these mappings, it's better to use PSE (4 MiB pages), to avoid
     * allocations for every page table (192 frames saved)
     */

    /* Direct mapping, 192 * 4 = 768 MiB */
    for (uint32_t i = 0; i < 192; i++) {
      pd[i + X86_KERNEL_PDE_BASE] = (i * 0x00400000) | X86_PDE_PRESENT |
                                    X86_PDE_WRITABLE | X86_PDE_PAGE_SIZE;
    }
  } else {
    /*
     * TODO: init user virtual space
     */
  }

  /* Set the CR3 to the physical address where the page directory is stored*/
  x86_cr3_write(pd_phys);

  return KERR_OK;
}
