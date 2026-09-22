#include <arch/cpu/control.h>
#include <arch/mm.h>
#include <base/align.h>
#include <base/bit.h>
#include <kernel/error.h>
#include <klib/memory.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

/*
 * This implementation uses only 4 KiB pages and does not use
 * 4 MiB pages through PSE
 *
 * TODO: Add support for 4MiB pages if needed
 */

#define X86_PAGE_SIZE 4096
#define X86_PDE_COUNT 1024
#define X86_KERNEL_PDE_BASE 768
#define X86_PTE_COUNT 1024
#define X86_PAGE_MASK 0xFFFFF000

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

size_t arch_vm_page_size(void) { return X86_PAGE_SIZE; }

/**
 * @brief Invalidate a single TLB entry
 *
 * @param va Virtual address whose translation must be invalidated
 */
static void x86_invlpg(vaddr_t va) {
  __asm__ __volatile__("invlpg (%0)" : : "r"(va) : "memory");
}

/**
 * @brief Convert generic VM flags to x86 PTE flags
 *
 * @param flags Generic virtual memory flags
 * @return Corresponding x86 PTE flags
 */
static uint32_t x86_vm_flags_to_pte(uint32_t flags) {
  uint32_t pte_flags = X86_PTE_PRESENT;

  if (flags & VM_WRITE)
    pte_flags |= X86_PTE_WRITABLE;

  if (flags & VM_USER)
    pte_flags |= X86_PTE_USER;

  return pte_flags;
}

/**
 * @brief Convert generic VM flags to x86 PDE flags
 *
 * PDEs are always present and writable
 * PTEs provide the per page write and user permissions
 *
 * @param flags Generic virtual memory flags
 * @return Corresponding x86 PDE flags
 */
static uint32_t x86_vm_flags_to_pde(uint32_t flags) {

  uint32_t pde_flags = X86_PDE_PRESENT | X86_PDE_WRITABLE;

  if (flags & VM_USER)
    pde_flags |= X86_PDE_USER;

  return pde_flags;
}

int32_t arch_vm_space_init(struct arch_vm_space *vm) {
  if (!vm)
    return -KERR_INVAL;

  paddr_t pd_phys;

  KERR_TRY(PMM_ALLOC_LOWMEM(1, &pd_phys));

  uint32_t *pd = (uint32_t *)PHYS_TO_VIRT(pd_phys);

  memset(pd, 0, X86_PAGE_SIZE);

  vm->pd_phys = pd_phys;

  return KERR_OK;
}

int32_t arch_vm_space_destroy(struct arch_vm_space *vm) {
  if (!vm)
    return -KERR_INVAL;

  uint32_t *pd = (uint32_t *)PHYS_TO_VIRT(vm->pd_phys);

  /*
   * Free page tables owned by this address space
   * Do not free shared kernel mappings
   */
  for (size_t i = 0; i < X86_KERNEL_PDE_BASE; i++) {
    uint32_t pde = pd[i];

    if (!(pde & X86_PDE_PRESENT))
      continue;

    if (pde & X86_PDE_PAGE_SIZE)
      continue;

    paddr_t pt_phys = pde & X86_PAGE_MASK;

    KERR_TRY(pmm_free(pt_phys, 1));
  }

  paddr_t pd_phys = vm->pd_phys;

  KERR_TRY(pmm_free(pd_phys, 1));

  vm->pd_phys = 0;
  return KERR_OK;
}

/**
 * Only the kernel PDEs are copied.
 *
 * PDEs contain the physical addresses of the page tables, so
 * every address space references the same kernel page tables.
 *
 * All kernel PDEs that are required by processes must already
 * exist in kernel_vm before this function is called.
 */
int32_t arch_vm_space_attach_kernel(struct arch_vm_space *vm,
                                    const struct arch_vm_space *kernel_vm) {
  if (!vm || !kernel_vm)
    return -KERR_INVAL;

  uint32_t *pd = (uint32_t *)PHYS_TO_VIRT(vm->pd_phys);

  const uint32_t *kernel_pd =
      (const uint32_t *)PHYS_TO_VIRT(kernel_vm->pd_phys);

  for (size_t i = X86_KERNEL_PDE_BASE; i < X86_PDE_COUNT; i++)
    pd[i] = kernel_pd[i];

  return KERR_OK;
}

/*
 * TODO: make this operation atomic
 */
int32_t arch_vm_map(struct arch_vm_space *vm, vaddr_t virt, paddr_t phys,
                    size_t size, uint32_t flags) {
  if (!vm || size == 0)
    return -KERR_INVAL;

  if (!ALIGN_TEST(virt, X86_PAGE_SIZE) || !ALIGN_TEST(phys, X86_PAGE_SIZE) ||
      !ALIGN_TEST(size, X86_PAGE_SIZE)) {
    return -KERR_INVAL;
  }

  uint32_t *pd = (uint32_t *)PHYS_TO_VIRT(vm->pd_phys);

  for (size_t off = 0; off < size; off += X86_PAGE_SIZE) {
    vaddr_t va = virt + off;
    paddr_t pa = phys + off;

    size_t pde_index = va >> 22;
    size_t pte_index = (va >> 12) & 0x3FF;

    uint32_t pde = pd[pde_index];

    /*
     * Allocate the page table when the PDE does not
     * already reference one
     */
    if (!(pde & X86_PDE_PRESENT)) {
      paddr_t pt_phys;

      KERR_TRY(PMM_ALLOC_LOWMEM(1, &pt_phys));

      uint32_t *pt = (uint32_t *)PHYS_TO_VIRT(pt_phys);

      memset(pt, 0, X86_PAGE_SIZE);

      pd[pde_index] = pt_phys | x86_vm_flags_to_pde(flags);

      pde = pd[pde_index];
    } else if (pde & X86_PDE_PAGE_SIZE) {
      return -KERR_NOSUP;
    }

    uint32_t *pt = (uint32_t *)PHYS_TO_VIRT(pde & X86_PAGE_MASK);

    if (pt[pte_index] & X86_PTE_PRESENT)
      return -KERR_EXISTS;

    pt[pte_index] = (pa & X86_PAGE_MASK) | x86_vm_flags_to_pte(flags);

    x86_invlpg(va);
  }

  return KERR_OK;
}

/* TODO:
 *  - make this operation atomic
 *  - clear the corresponding PDE and and free the page table if it is empty
 */
int32_t arch_vm_unmap(struct arch_vm_space *vm, vaddr_t virt, size_t size) {
  if (!vm || size == 0)
    return -KERR_INVAL;

  if (!ALIGN_TEST(virt, X86_PAGE_SIZE) || !ALIGN_TEST(size, X86_PAGE_SIZE)) {
    return -KERR_INVAL;
  }

  uint32_t *pd = (uint32_t *)PHYS_TO_VIRT(vm->pd_phys);

  for (size_t off = 0; off < size; off += X86_PAGE_SIZE) {
    vaddr_t va = virt + off;

    size_t pde_index = va >> 22;
    size_t pte_index = (va >> 12) & 0x3FF;

    uint32_t pde = pd[pde_index];

    if (!(pde & X86_PDE_PRESENT))
      return -KERR_NOENT;

    if (pde & X86_PDE_PAGE_SIZE)
      return -KERR_NOSUP;

    uint32_t *pt = (uint32_t *)PHYS_TO_VIRT(pde & X86_PAGE_MASK);

    if (!(pt[pte_index] & X86_PTE_PRESENT))
      return -KERR_NOENT;

    pt[pte_index] = 0;

    x86_invlpg(va);
  }

  return KERR_OK;
}

int32_t arch_vm_protect(struct arch_vm_space *vm, vaddr_t virt, size_t size,
                        uint32_t flags) {
  if (!vm || size == 0)
    return -KERR_INVAL;

  if (!ALIGN_TEST(virt, X86_PAGE_SIZE) || !ALIGN_TEST(size, X86_PAGE_SIZE)) {
    return -KERR_INVAL;
  }

  uint32_t *pd = (uint32_t *)PHYS_TO_VIRT(vm->pd_phys);

  for (size_t off = 0; off < size; off += X86_PAGE_SIZE) {
    vaddr_t va = virt + off;

    size_t pde_index = va >> 22;
    size_t pte_index = (va >> 12) & 0x3FF;

    uint32_t pde = pd[pde_index];

    if (!(pde & X86_PDE_PRESENT))
      return -KERR_NOENT;

    if (pde & X86_PDE_PAGE_SIZE)
      return -KERR_NOSUP;

    uint32_t *pt = (uint32_t *)PHYS_TO_VIRT(pde & X86_PAGE_MASK);

    uint32_t pte = pt[pte_index];

    if (!(pte & X86_PTE_PRESENT))
      return -KERR_NOENT;

    paddr_t phys = pte & X86_PAGE_MASK;

    pt[pte_index] = phys | x86_vm_flags_to_pte(flags);

    x86_invlpg(va);
  }

  return KERR_OK;
}

int32_t arch_vm_space_load(struct arch_vm_space *vm) {
  if (!vm)
    return -KERR_INVAL;
  x86_cr3_write(vm->pd_phys);
  return KERR_OK;
}