#include "kernel/printk.h"
#include <arch/x86/cpu/cpuid.h>
#include <arch/x86/cpu/flags.h>
#include <base/bit.h>

struct x86_cpuid_regs {
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
};

struct x86_cpu_info x86_cpu_info;

#define CPUINFO_LOG "CPUINFO: "

static bool x86_cpuid_supported(void) {
  uint32_t old = x86_eflags_read();

  uint32_t test = old ^ X86_FLAGS_ID_MASK; // toggle ID flag

  x86_eflags_write(test);

  uint32_t now = x86_eflags_read();

  x86_eflags_write(old);

  /*
   * If the CPU was able to change the ID flag,
   * the CPUID instruction is supported.
   */
  return MASK_TEST(old ^ now, X86_FLAGS_ID_MASK);
}

static void x86_cpuid(uint32_t leaf, uint32_t subleaf,
                      struct x86_cpuid_regs *out) {
  uint32_t eax = leaf;
  uint32_t ebx;
  uint32_t ecx = subleaf;
  uint32_t edx;

  __asm__ volatile("cpuid" : "+a"(eax), "=b"(ebx), "+c"(ecx), "=d"(edx));

  out->eax = eax;
  out->ebx = ebx;
  out->ecx = ecx;
  out->edx = edx;
}

bool x86_cpu_detect(void) {
  if (!x86_cpuid_supported())
    return false;

  struct x86_cpuid_regs regs;

  /*
   * EAX = max basic leaf
   * EBX = vendor[0..3]
   * EDX = vendor[4..7]
   * ECX = vendor[8..11]
   */
  x86_cpuid(0, 0, &regs);

  x86_cpu_info.max_basic_leaf = regs.eax;
  *(uint32_t *)(x86_cpu_info.vendor) = regs.ebx;
  *(uint32_t *)(x86_cpu_info.vendor + 4) = regs.edx;
  *(uint32_t *)(x86_cpu_info.vendor + 8) = regs.ecx;

  pr_debug(CPUINFO_LOG "Vendor (%s)\n", x86_cpu_info.vendor);

  return true;
}