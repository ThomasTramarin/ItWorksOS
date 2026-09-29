#include <kernel/error.h>
#include <mm/kernel_stack.h>
#include <mm/kheap.h>
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
