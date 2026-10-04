#include <arch/mm.h>
#include <arch/proc.h>
#include <base/stdint.h>
#include <format/iwbf.h>
#include <kernel/error.h>
#include <klib/memory.h>
#include <mm/kernel_stack.h>
#include <mm/kheap.h>
#include <mm/layout.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <process/process.h>

static struct process *process_table[PROCESS_MAX];

struct process *process_get(pid_t pid) {
  if (pid < PID_FIRST || pid >= PID_MAX)
    return NULL;

  return process_table[pid];
}

struct process *process_create(const char *name) {
  if (!name)
    return KERR_PTR(-KERR_INVAL);

  pid_t pid = pid_alloc();

  if (KERR_IS_ERR(pid))
    return KERR_PTR(pid);

  struct process *proc = kmalloc(sizeof(*proc));
  if (KERR_PTR_IS_ERR(proc)) {
    pid_free(pid);
    return proc;
  }

  memset(proc, 0, sizeof(*proc));

  proc->pid = pid;
  proc->state = PROCESS_CREATED;
  proc->name = name;

  int32_t err;

  err = vm_space_init(&proc->vm);
  if (KERR_IS_ERR(err)) {
    pid_free(pid);
    kfree(proc);
    return KERR_PTR(err);
  }

  err = kstack_alloc(&proc->kstack);

  if (KERR_IS_ERR(err)) {
    vm_space_destroy(&proc->vm);
    pid_free(pid);
    kfree(proc);
    return KERR_PTR(err);
  }

  process_table[pid] = proc;

  return proc;
}

/*
 * TODO: better checks
 */
int32_t process_load(struct process *proc, const void *image, size_t size) {
  if (!proc || !image || size == 0)
    return -KERR_INVAL;

  if (proc->state != PROCESS_CREATED)
    return -KERR_INVAL;

  size_t page_size = arch_vm_page_size();

  struct iwbf32_hdr hdr;
  KERR_TRY(iwbf_parse_hdr(image, size, &hdr));

  for (uint32_t i = 0; i < hdr.segment_count; i++) {

    size_t seg_off = IWBF32_HDR_SIZE + IWBF32_SEGMENT_HDR_SIZE * i;

    struct iwbf32_segment_hdr seg;

    KERR_TRY(iwbf_parse_segment_hdr((const uint8_t *)image + seg_off,
                                    size - seg_off, &seg));

    /*
     * Basic checks
     */
    if (seg.file_size > seg.mem_size)
      return -KERR_INVAL;

    if (seg.file_off > size)
      return -KERR_INVAL;

    if (seg.file_size > size - seg.file_off)
      return -KERR_INVAL;

    /*
     * Convert IWBF permissions to VMA permissions
     */
    uint32_t vm_flags = VM_USER;

    if (seg.flags & IWBF_SEG_FLAG_READ) {
      vm_flags |= VM_READ;
    }

    if (seg.flags & IWBF_SEG_FLAG_WRITE) {
      vm_flags |= VM_WRITE;
    }

    if (seg.flags & IWBF_SEG_FLAG_EXEC) {
      vm_flags |= VM_EXEC;
    }

    /*
     * Allocate physical memory for the segment
     */
    paddr_t phys;
    KERR_TRY(PMM_ALLOC_LOWMEM(seg.mem_size / page_size, &phys));

    /* Create VMA */
    int32_t err =
        vm_map(&proc->vm, (vaddr_t)seg.vaddr, phys, seg.mem_size, vm_flags);

    if (KERR_IS_ERR(err)) {
      pmm_free(phys, seg.mem_size / page_size);
      return err;
    }

    /* Copy file data */
    if (seg.file_size != 0) {
      memcpy((void *)PHYS_TO_VIRT(phys), image + seg.file_off, seg.file_size);
    }

    /* Fill the remaining part of the segment with zero bytes*/
    if (seg.mem_size > seg.file_size) {
      memset((uint8_t *)PHYS_TO_VIRT(phys) + seg.file_size, 0,
             seg.mem_size - seg.file_size);
    }
  }

  proc->entry = hdr.entry;

  return KERR_OK;
}

int32_t process_prepare(struct process *proc) {
  if (!proc)
    return -KERR_INVAL;

  size_t page_size = arch_vm_page_size();

  if (proc->state != PROCESS_CREATED)
    return -KERR_INVAL;

  /* Allocate user stack */
  paddr_t stack_phys;

  KERR_TRY(PMM_ALLOC_LOWMEM(USER_STACK_SIZE / page_size, &stack_phys));

  KERR_TRY(vm_map(&proc->vm, USER_STACK_TOP - USER_STACK_SIZE, stack_phys,
                  USER_STACK_SIZE,
                  VM_USER | VM_READ | VM_WRITE | VM_GROWSDOWN));

  KERR_TRY(arch_process_stack_prepare(proc));

  return KERR_OK;
}

int32_t process_start(struct process *proc) {
  if (!proc)
    return -KERR_INVAL;

  arch_process_start(proc);

  __builtin_unreachable();
}