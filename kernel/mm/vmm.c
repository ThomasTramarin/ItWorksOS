#include <arch/mm.h>
#include <base/align.h>
#include <base/limits.h>
#include <base/sections.h>
#include <base/stddef.h>
#include <kernel/error.h>
#include <klib/list.h>
#include <klib/memory.h>
#include <mm/kernel_vm.h>
#include <mm/kheap.h>
#include <mm/layout.h>
#include <mm/vmm.h>

#define VM_USER_START USER_START
#define VM_USER_END (USER_START + USER_SIZE)

static bool vm_user_range_valid(vaddr_t virt, size_t size) {
  if (size == 0)
    return false;

  /*
   * User virtual addresses must remain entirely below
   * the kernel virtual address space.
   */
  if (virt < VM_USER_START || virt >= VM_USER_END)
    return false;

  if (size - 1 > UINTPTR_MAX - virt)
    return false;

  vaddr_t end = virt + size;

  return end <= VM_USER_END;
}

static bool vm_phys_range_valid(paddr_t phys, size_t size) {
  if (size == 0)
    return false;

  if (size - 1 > UINT64_MAX - phys)
    return false;

  return true;
}

static struct vma *vm_find_vma_range(struct vm_space *vm, vaddr_t virt,
                                     size_t size) {
  if (!vm || !vm_user_range_valid(virt, size))
    return NULL;

  vaddr_t end = virt + size;

  struct list_node *node;

  list_for_each(node, &vm->vmas) {
    struct vma *vma = container_of(node, struct vma, vmas_node);

    if (vma->start > virt)
      break;

    if (vma->start <= virt && end <= vma->end)
      return vma;
  }

  return NULL;
}

struct vma *vm_find_vma(struct vm_space *vm, vaddr_t virt) {
  if (!vm || virt >= VM_USER_END)
    return NULL;

  struct list_node *node;

  list_for_each(node, &vm->vmas) {
    struct vma *vma = container_of(node, struct vma, vmas_node);

    if (vma->start > virt)
      break;

    if (vma->start <= virt && virt < vma->end)
      return vma;
  }

  return NULL;
}

static int32_t vm_add_vma(struct vm_space *vm, vaddr_t virt, size_t size,
                          uint32_t flags) {
  if (!vm || size == 0)
    return -KERR_INVAL;

  size_t page_size = arch_vm_page_size();

  if (!ALIGN_TEST(virt, page_size) || !ALIGN_TEST(size, page_size))
    return -KERR_INVAL;

  if (!vm_user_range_valid(virt, size))
    return -KERR_INVAL;

  vaddr_t new_end = virt + size;

  struct list_node *node;

  list_for_each(node, &vm->vmas) {
    struct vma *vma = container_of(node, struct vma, vmas_node);

    /*
     * Existing VMA is entirely before the new one
     */
    if (vma->end <= virt)
      continue;

    /*
     * Existing VMA is entirely after the new one
     */
    if (vma->start >= new_end)
      break;

    /*
     * Ranges overlap
     */
    return -KERR_EXISTS;
  }

  struct vma *new_vma = kmalloc(sizeof(*new_vma));

  if (KERR_PTR_IS_ERR(new_vma))
    return -KERR_NOMEM;

  new_vma->start = virt;
  new_vma->end = new_end;
  new_vma->flags = flags;

  list_insert_before(node, &new_vma->vmas_node);

  return KERR_OK;
}

static int32_t vm_remove_vma(struct vm_space *vm, vaddr_t virt) {
  struct vma *vma = vm_find_vma(vm, virt);

  if (!vma)
    return -KERR_NOENT;

  list_remove(&vma->vmas_node);
  kfree(vma);

  return KERR_OK;
}

int32_t vm_map(struct vm_space *vm, vaddr_t virt, paddr_t phys, size_t size,
               uint32_t flags) {
  if (!vm || size == 0)
    return -KERR_INVAL;

  size_t page_size = arch_vm_page_size();

  if (!ALIGN_TEST(virt, page_size) || !ALIGN_TEST(phys, page_size) ||
      !ALIGN_TEST(size, page_size))
    return -KERR_INVAL;

  if (!vm_user_range_valid(virt, size))
    return -KERR_INVAL;

  if (!vm_phys_range_valid(phys, size))
    return -KERR_INVAL;

  /*
   * Create the software VMA first
   *
   * vm_add_vma() guarantees that the virtual range does not
   * overlap an existing VMA
   */
  KERR_TRY(vm_add_vma(vm, virt, size, flags));

  /*
   * Create the architecture-specific page table mappings
   */
  int32_t err = arch_vm_map(&vm->arch, virt, phys, size, flags);

  if (err < 0) {
    vm_remove_vma(vm, virt);
    return err;
  }

  return KERR_OK;
}

int32_t vm_unmap(struct vm_space *vm, vaddr_t virt, size_t size) {
  if (!vm || size == 0)
    return -KERR_INVAL;

  size_t page_size = arch_vm_page_size();

  if (!ALIGN_TEST(virt, page_size) || !ALIGN_TEST(size, page_size))
    return -KERR_INVAL;

  if (!vm_user_range_valid(virt, size))
    return -KERR_INVAL;

  struct vma *vma = vm_find_vma_range(vm, virt, size);

  if (!vma)
    return -KERR_NOENT;

  /*
   * VMA splitting is not implemented yet
   *
   * For now, only an entire VMA can be unmapped
   */
  if (vma->start != virt || vma->end != virt + size)
    return -KERR_INVAL;

  KERR_TRY(arch_vm_unmap(&vm->arch, virt, size));

  list_remove(&vma->vmas_node);
  kfree(vma);

  return KERR_OK;
}

int32_t vm_protect(struct vm_space *vm, vaddr_t virt, size_t size,
                   uint32_t flags) {
  if (!vm || size == 0)
    return -KERR_INVAL;

  size_t page_size = arch_vm_page_size();

  if (!ALIGN_TEST(virt, page_size) || !ALIGN_TEST(size, page_size))
    return -KERR_INVAL;

  if (!vm_user_range_valid(virt, size))
    return -KERR_INVAL;

  struct vma *vma = vm_find_vma_range(vm, virt, size);

  if (!vma)
    return -KERR_NOENT;

  /*
   * VMA splitting is not implemented yet
   *
   * For now, protection can only be changed for an entire VMA
   */
  if (vma->start != virt || vma->end != virt + size)
    return -KERR_INVAL;

  KERR_TRY(arch_vm_protect(&vm->arch, virt, size, flags));

  vma->flags = flags;

  return KERR_OK;
}

int32_t vm_space_init(struct vm_space *vm) {
  if (!vm)
    return -KERR_INVAL;

  list_init(&vm->vmas);

  KERR_TRY(arch_vm_space_init(&vm->arch));

  /*
   * Attach the shared kernel page tables
   */
  int32_t err = vm_kern_attach(&vm->arch);

  if (err < 0) {
    arch_vm_space_destroy(&vm->arch);
    return err;
  }

  return KERR_OK;
}

int32_t vm_space_destroy(struct vm_space *vm) {
  if (!vm)
    return -KERR_INVAL;

  struct list_node *node;
  struct list_node *next;

  /*
   * VMA metadata is owned by the vm_space
   */
  list_for_each_safe(node, next, &vm->vmas) {
    struct vma *vma = container_of(node, struct vma, vmas_node);

    list_remove(&vma->vmas_node);
    kfree(vma);
  }

  /*
   * The architecture layer destroys the private page tables
   * and page directory. It does not free the physical frames
   * referenced by user mappings.
   */
  return arch_vm_space_destroy(&vm->arch);
}

int32_t vm_space_load(struct vm_space *vm) {
  if (!vm)
    return -KERR_INVAL;

  return arch_vm_space_load(&vm->arch);
}

int32_t __init vmm_init(void) {
  KERR_TRY(vm_kern_init());

  return KERR_OK;
}