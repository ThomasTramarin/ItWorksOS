#include <arch/cpu/cpuid.h>
#include <arch/cpu/flags.h>
#include <base/bit.h>
#include <kernel/printk.h>
#include <klib/memory.h>

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

static void x86_cpuid_leaf_1_decode(const struct x86_cpuid_regs *regs) {
  uint32_t eax = regs->eax;
  uint32_t ebx = regs->ebx;

  struct x86_cpuid_leaf_1 *leaf = &x86_cpu_info.leaf1;

  /*
   * EAX
   *
   * [3:0]   Stepping
   * [7:4]   Model
   * [11:8]  Family
   * [13:12] Processor Type
   * [19:16] Extended Model
   * [27:20] Extended Family
   */
  leaf->stepping = eax & 0x0f;
  leaf->model = (eax >> 4) & 0x0f;
  leaf->family = (eax >> 8) & 0x0f;
  leaf->type = (eax >> 12) & 0x03;

  leaf->extended_model = (eax >> 16) & 0x0f;
  leaf->extended_family = (eax >> 20) & 0xff;

  /*
   * EBX
   *
   * [7:0]   Brand Index
   * [15:8]  CLFLUSH line size, in units of 8 bytes
   * [23:16] Maximum number of logical processors
   * [31:24] Initial APIC ID
   */

  leaf->clflush_size = ((ebx >> 8) & 0xff) * 8;

  leaf->logical_processors = (ebx >> 16) & 0xff;

  leaf->initial_apic_id = (ebx >> 24) & 0xff;

  /*
   * ECX / EDX
   *
   * Keep feature bits as raw bitmasks.
   */
  leaf->features_ecx = regs->ecx;
  leaf->features_edx = regs->edx;

  leaf->valid = true;
}

bool x86_cpu_detect(void) {

  memset(&x86_cpu_info, 0, sizeof(x86_cpu_info));

  if (!x86_cpuid_supported())
    return false;

  struct x86_cpuid_regs regs;

  /*
   * CPUID.0H
   *
   * EAX = max basic leaf
   * EBX = vendor[0..3]
   * EDX = vendor[4..7]
   * ECX = vendor[8..11]
   */
  x86_cpuid(0, 0, &regs);

  x86_cpu_info.max_basic_leaf = regs.eax;

  // vendor
  memcpy(x86_cpu_info.vendor + 0, &regs.ebx, 4);
  memcpy(x86_cpu_info.vendor + 4, &regs.edx, 4);
  memcpy(x86_cpu_info.vendor + 8, &regs.ecx, 4);
  x86_cpu_info.vendor[12] = '\0';

  /*
   * CPUID.80000000H
   *
   * EAX = maximum supported extended leaf.
   */
  x86_cpuid(0x80000000, 0, &regs);

  x86_cpu_info.max_extended_leaf = regs.eax;

  pr_debug(CPUINFO_LOG
           "Vendor (%s), Max basic leaf (0x%x), Max extended leaf (0x%x)\n",
           x86_cpu_info.vendor, x86_cpu_info.max_basic_leaf,
           x86_cpu_info.max_extended_leaf);

  /*
   * CPUID.01H
   *
   * Processor information and feature bits.
   */
  if (x86_cpu_info.max_basic_leaf >= 1) {
    x86_cpuid(1, 0, &regs);

    x86_cpuid_leaf_1_decode(&regs);

    pr_debug(CPUINFO_LOG "Family (0x%x), Model (0x%x), Stepping (0x%x)\n",
             x86_cpu_info.leaf1.family, x86_cpu_info.leaf1.model,
             x86_cpu_info.leaf1.stepping);

    pr_debug(CPUINFO_LOG "Logical processors (%u), Initial APIC ID (%u)\n",
             x86_cpu_info.leaf1.logical_processors,
             x86_cpu_info.leaf1.initial_apic_id);

    pr_debug(CPUINFO_LOG "CLFLUSH line size (%uB)\n",
             x86_cpu_info.leaf1.clflush_size);

    pr_debug(CPUINFO_LOG "CPUID.01H.ECX (0x%x), CPUID.01H.EDX (0x%x)\n",
             x86_cpu_info.leaf1.features_ecx, x86_cpu_info.leaf1.features_edx);
  }

  return true;
}