#include <arch/mm.h>
#include <kernel/error.h>
#include <klib/bitmap.h>
#include <mm/kernel_stack.h>
#include <mm/kernel_vm.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

#define KERNEL_STACK_MAX 64

static uint32_t kernel_stack_bm_data[BITMAP_ELEMS(KERNEL_STACK_MAX)];
static struct bitmap kernel_stack_bm =
    BITMAP_INIT(kernel_stack_bm_data, KERNEL_STACK_MAX);

int32_t kstack_alloc(struct kernel_stack *stack) {
  if (!stack)
    return -KERR_INVAL;

  if (stack->base != 0 || stack->size != 0)
    return -KERR_EXISTS;

  size_t pos;

  if (bitmap_find_zero(&kernel_stack_bm, &pos) == false) {
    return -KERR_NOSPC;
  }

  size_t page_size = arch_vm_page_size();

  vaddr_t base = KERNEL_STACK_BASE + KERNEL_STACK_PAGES * pos * page_size;

  paddr_t frames[KERNEL_STACK_PAGES];

  for (size_t i = 0; i < KERNEL_STACK_PAGES; i++) {
    KERR_TRY(PMM_ALLOC_ANY(1, &frames[i]));

    KERR_TRY(vm_kern_map(base + i * page_size, frames[i], page_size,
                         VM_GROWSDOWN | VM_READ | VM_WRITE));
  }

  bitmap_set(&kernel_stack_bm, pos);

  stack->base = base;
  stack->size = KERNEL_STACK_PAGES * arch_vm_page_size();

  return KERR_OK;
}

int32_t kstack_free(struct kernel_stack *stack) {
  if (!stack)
    return -KERR_INVAL;

  if (stack->base == 0 || stack->size == 0)
    return -KERR_NOENT;

  size_t page_size = arch_vm_page_size();

  size_t pos = (stack->base - KERNEL_STACK_BASE) /
               (KERNEL_STACK_PAGES * arch_vm_page_size());

  for (size_t i = 0; i < KERNEL_STACK_PAGES; i++) {
    vaddr_t virt = stack->base + i * page_size;
    paddr_t phys;

    KERR_TRY(vm_kern_translate(virt, &phys));
    KERR_TRY(vm_kern_unmap(stack->base + i * page_size, page_size));

    KERR_TRY(pmm_free(phys, 1));
  }

  bitmap_clear(&kernel_stack_bm, pos);

  stack->base = 0;
  stack->size = 0;
  stack->stack_pointer = 0;

  return KERR_OK;
}