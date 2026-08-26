#ifndef KERNEL_EXCEPTION_H
#define KERNEL_EXCEPTION_H

#include <base/stdbool.h>
#include <base/stdint.h>

enum exception_type {
  EXCEPTION_ARITHMETIC,
  EXCEPTION_UNKNOWN,
};

/**
 * @brief Generic information about a CPU exception
 *
 * This structure contains architecture-independent information
 * extracted from an architecture-specific exception frame.
 *
 * The arch_ctx field contains the original architecture-specific
 * context
 */
struct exception_info {
  enum exception_type type;
  uintptr_t ip; // Instruction Pointer
  bool user_mode;
  void *arch_ctx; // arch-specific interrupt frame
};

/**
 * @brief Kernel-level CPU exception entry point
 *
 * This function receives an architecture-independent exception
 * description and decides how the kernel should handle it.
 */
void exception_dispatch(struct exception_info *info);

#endif