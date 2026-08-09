#ifndef ARCH_X86_CPU_CPUID_H
#define ARCH_X86_CPU_CPUID_H

#include <base/stdbool.h>

// CPUID.01H - CPU info and feature bits
struct x86_cpuid_leaf_1 {
  bool valid;

  uint8_t stepping;
  uint8_t model;
  uint8_t family;
  uint8_t type;
  uint8_t extended_model;
  uint8_t extended_family;

  uint8_t clflush_size;
  uint8_t logical_processors;
  uint8_t initial_apic_id;

  uint32_t features_ecx;
  uint32_t features_edx;
};

struct x86_cpu_info {
  // CPU identification
  char vendor[13]; // 12 + null term

  // CPUID availability
  uint32_t max_basic_leaf;
  uint32_t max_extended_leaf;

  struct x86_cpuid_leaf_1 leaf1;
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
