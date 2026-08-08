#ifndef ARCH_X86_CPU_CPUID_H
#define ARCH_X86_CPU_CPUID_H

#include <base/stdbool.h>

struct x86_cpu_info {
  char vendor[13]; // 12 + null term
  uint32_t max_basic_leaf;
};

extern struct x86_cpu_info x86_cpu_info;

/**
 * @brief Get x86 CPU information and store it into x86_cpu_info
 *
 * Returns true if the CPUID instruction is supported and x86_cpu_info
 * contains data, false otherwise.
 */
bool x86_cpu_detect(void);

#endif
