#include "syscall/syscall.h"
#include <arch/interrupts/syscall.h>

void x86_syscall_handler(struct x86_interrupt_frame *frame) {

  struct syscall_args args = {
      .number = frame->eax,
      .arg1 = frame->ebx,
      .arg2 = frame->ecx,
      .arg3 = frame->edx,
      .arg4 = frame->esi,
      .arg5 = frame->edi,
  };

  frame->eax = syscall_dispatch(&args);
}