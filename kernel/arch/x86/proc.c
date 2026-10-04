#include <arch/cpu/flags.h>
#include <arch/cpu/gdt.h>
#include <arch/cpu/tss.h>
#include <arch/proc.h>
#include <base/compiler.h>
#include <kernel/error.h>
#include <process/process.h>

int32_t arch_process_stack_prepare(struct process *proc) {
  if (!proc)
    return -KERR_INVAL;

  vaddr_t top = proc->kstack.base + proc->kstack.size;
  uint32_t *sp = (uint32_t *)top;

  *--sp = X86_GDT_SELECTOR_USER_DATA;                  // SS
  *--sp = USER_STACK_TOP;                              // ESP
  *--sp = X86_FLAGS_IF_MASK | X86_FLAGS_RESERVED_MASK; // EFLAGS
  *--sp = X86_GDT_SELECTOR_USER_CODE;                  // CS
  *--sp = proc->entry;                                 // EIP

  proc->kstack.stack_pointer = (vaddr_t)sp;

  return KERR_OK;
}

void __noreturn arch_process_start(struct process *proc) {

  uint32_t stack_top = proc->kstack.base + proc->kstack.size;
  x86_tss_set_kernel_stack(stack_top);

  uint32_t stack_pointer = proc->kstack.stack_pointer;
  asm volatile("movl %0, %%esp\n"
               "iret\n"
               :
               : "r"(stack_pointer)
               : "memory");
}