#include <arch/mm.h>
#include <base/sections.h>
#include <base/stdbool.h>
#include <kernel/error.h>
#include <mm/kernel_vm.h>
#include <mm/layout.h>
#include <mm/vmm.h>

/**
 * @brief Kernel virtual mappings
 *
 * The kernel is not a vm_space because it does not contain VMAs
 * The kernel_vm only contains page mappings
 */
static struct arch_vm_space kernel_vm;

static bool vm_kern_range_valid(vaddr_t virt, size_t size) {
  if (size == 0)
    return false;

  if (virt < KERNEL_VIRT_OFFSET)
    return false;

  if (size - 1 > UINTPTR_MAX - virt)
    return false;

  return true;
}

static int32_t vm_kern_map(vaddr_t virt, paddr_t phys, size_t size,
                           uint32_t flags) {
  if (!vm_kern_range_valid(virt, size))
    return -KERR_INVAL;

  return arch_vm_map(&kernel_vm, virt, phys, size, flags);
}

static int32_t vm_kern_protect(vaddr_t virt, size_t size, uint32_t flags) {
  if (!vm_kern_range_valid(virt, size))
    return -KERR_INVAL;

  return arch_vm_protect(&kernel_vm, virt, size, flags);
}

int32_t vm_kern_attach(struct arch_vm_space *vm) {
  if (!vm)
    return -KERR_INVAL;

  return arch_vm_space_attach_kernel(vm, &kernel_vm);
}

int32_t __init vm_kern_init(void) {

  /**
   * Initialize the master kernel address space
   *
   * The kernel virtual layout is created during system initialization
   * and remains fixed for the lifetime of the system
   *
   * Each process address space will share the same kernel mappings
   * when it is created
   */
  KERR_TRY(arch_vm_space_init(&kernel_vm));

  /**
   * Create the permanent direct map
   */
  KERR_TRY(vm_kern_map(KERNEL_VIRT_OFFSET, 0, KERNEL_DIRECT_MAP_SIZE,
                       VM_READ | VM_WRITE));

  /**
   * Make kernel code read-only
   */
  KERR_TRY(vm_kern_protect((vaddr_t)__text_start,
                           (size_t)(__text_end - __text_start),
                           VM_READ | VM_EXEC));

  /**
   * Make read-only data read-only
   */
  KERR_TRY(vm_kern_protect((vaddr_t)__rodata_start,
                           (size_t)(__rodata_end - __rodata_start), VM_READ));

  KERR_TRY(arch_vm_space_load(&kernel_vm));

  return KERR_OK;
}