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

#define X86_CPUID_1_ECX_SSE3 BIT(0)
#define X86_CPUID_1_ECX_PCLMULQDQ BIT(1)
#define X86_CPUID_1_ECX_DTES64 BIT(2)
#define X86_CPUID_1_ECX_MONITOR BIT(3)
#define X86_CPUID_1_ECX_DS_CPL BIT(4)
#define X86_CPUID_1_ECX_VMX BIT(5)
#define X86_CPUID_1_ECX_SMX BIT(6)
#define X86_CPUID_1_ECX_EST BIT(7)
#define X86_CPUID_1_ECX_TM2 BIT(8)
#define X86_CPUID_1_ECX_SSSE3 BIT(9)
#define X86_CPUID_1_ECX_CNXT_ID BIT(10)
#define X86_CPUID_1_ECX_SDBG BIT(11)
#define X86_CPUID_1_ECX_FMA BIT(12)
#define X86_CPUID_1_ECX_CX16 BIT(13)
#define X86_CPUID_1_ECX_XTPR BIT(14)
#define X86_CPUID_1_ECX_PDCM BIT(15)
#define X86_CPUID_1_ECX_PCID BIT(17)
#define X86_CPUID_1_ECX_DCA BIT(18)
#define X86_CPUID_1_ECX_SSE4_1 BIT(19)
#define X86_CPUID_1_ECX_SSE4_2 BIT(20)
#define X86_CPUID_1_ECX_X2APIC BIT(21)
#define X86_CPUID_1_ECX_MOVBE BIT(22)
#define X86_CPUID_1_ECX_POPCNT BIT(23)
#define X86_CPUID_1_ECX_TSC_DEADLINE BIT(24)
#define X86_CPUID_1_ECX_AES BIT(25)
#define X86_CPUID_1_ECX_XSAVE BIT(26)
#define X86_CPUID_1_ECX_OSXSAVE BIT(27)
#define X86_CPUID_1_ECX_AVX BIT(28)
#define X86_CPUID_1_ECX_F16C BIT(29)
#define X86_CPUID_1_ECX_RDRAND BIT(30)

#define X86_CPUID_1_EDX_FPU BIT(0)
#define X86_CPUID_1_EDX_VME BIT(1)
#define X86_CPUID_1_EDX_DE BIT(2)
#define X86_CPUID_1_EDX_PSE BIT(3)
#define X86_CPUID_1_EDX_TSC BIT(4)
#define X86_CPUID_1_EDX_MSR BIT(5)
#define X86_CPUID_1_EDX_PAE BIT(6)
#define X86_CPUID_1_EDX_MCE BIT(7)
#define X86_CPUID_1_EDX_CX8 BIT(8)
#define X86_CPUID_1_EDX_APIC BIT(9)
#define X86_CPUID_1_EDX_SEP BIT(11)
#define X86_CPUID_1_EDX_MTRR BIT(12)
#define X86_CPUID_1_EDX_PGE BIT(13)
#define X86_CPUID_1_EDX_MCA BIT(14)
#define X86_CPUID_1_EDX_CMOV BIT(15)
#define X86_CPUID_1_EDX_PAT BIT(16)
#define X86_CPUID_1_EDX_PSE36 BIT(17)
#define X86_CPUID_1_EDX_PSN BIT(18)
#define X86_CPUID_1_EDX_CLFLUSH BIT(19)
#define X86_CPUID_1_EDX_DS BIT(21)
#define X86_CPUID_1_EDX_ACPI BIT(22)
#define X86_CPUID_1_EDX_MMX BIT(23)
#define X86_CPUID_1_EDX_FXSR BIT(24)
#define X86_CPUID_1_EDX_SSE BIT(25)
#define X86_CPUID_1_EDX_SSE2 BIT(26)
#define X86_CPUID_1_EDX_SS BIT(27)
#define X86_CPUID_1_EDX_HTT BIT(28)
#define X86_CPUID_1_EDX_TM BIT(29)
#define X86_CPUID_1_EDX_PBE BIT(31)

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
