#ifndef PROCESS_PROCESS_H
#define PROCESS_PROCESS_H

#include <klib/list.h>
#include <mm/kernel_stack.h>
#include <mm/vmm.h>
#include <process/pid.h>

#define PROCESS_MAX PID_MAX

#define USER_STACK_TOP 0xBFFFE000
#define USER_STACK_SIZE (4 * 4096)

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

  vaddr_t entry;

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

/**
 * @brief Prepare a created process for execution
 *
 * Allocates the user stack and prepares the initial user mode
 * execution stack frame.
 *
 * @param proc Process to prepare
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t process_prepare(struct process *proc);

/**
 * @brief Get a process by its process PID
 *
 * @param pid Process ID
 * @return Pointer to the process, or NULL if not found
 */
struct process *process_get(pid_t pid);

/**
 * @brief Load an executable image into a process
 *
 * Loads the executable segments into the process address space
 * and sets the process entry point.
 *
 * @param proc Process to load the executable into
 * @param image Executable image previously loaded into RAM
 * @param size Size of the executable image in bytes
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t process_load(struct process *proc, const void *image, size_t size);

/**
 * @brief Start a prepared process
 *
 * Switches execution to the process in user mode.
 *
 * @param proc Process to start
 */
int32_t process_start(struct process *proc);

#endif