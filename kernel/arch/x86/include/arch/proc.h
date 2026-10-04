#ifndef ARCH_X86_PROC_H
#define ARCH_X86_PROC_H

#include <base/stdint.h>
struct process;

/**
 * @brief Prepare the initial user mode stack frame
 *
 * Builds the stack frame required by iret to enter the process
 * in user mode.
 *
 * @param proc Process to prepare
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t arch_process_stack_prepare(struct process *proc);

/**
 * @brief Start a process in user mode
 *
 * Sets the kernel stack used by the TSS and transfers execution
 * to the process using iret.
 *
 * @param proc Process to start
 */
void arch_process_start(struct process *proc);

#endif