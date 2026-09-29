#ifndef PROCESS_PROCESS_H
#define PROCESS_PROCESS_H

#include <klib/list.h>
#include <mm/kernel_stack.h>
#include <mm/vmm.h>
#include <process/pid.h>

#define PROCESS_MAX PID_MAX

enum process_state {
  PROCESS_CREATED,
  PROCESS_READY,
  PROCESS_RUNNING,
  PROCESS_BLOCKED,
  PROCESS_EXITED
};

struct process {
  const char *name;
  pid_t pid;
  enum process_state state;

  struct vm_space vm;

  struct kernel_stack kstack;

  struct list_node sched_node;
};

/**
 * @brief Create a new process in the CREATED state
 *
 * Allocates and initializes a new process with the given name.
 * The process is left in the PROCESS_CREATED state and is not
 * yet ready to be scheduled.
 *
 * @param name Process name
 * @return Pointer to the created process, or an encoded negative
 * kernel error code on failure
 */
struct process *process_create(const char *name);
struct process *process_get(pid_t pid);

#endif