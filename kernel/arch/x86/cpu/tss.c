#include <arch/cpu/gdt.h>
#include <base/sections.h>
#include <base/stdint.h>

#include <arch/cpu/tss.h>
#include <klib/memory.h>

static struct x86_tss tss;

void __init x86_tss_init(void) {
  memset(&tss, 0, sizeof(tss));

  tss.ss0 = X86_GDT_SELECTOR_KERN_DATA;

  tss.iomap_base = sizeof(tss);
}

struct x86_tss *x86_tss_get(void) { return &tss; }