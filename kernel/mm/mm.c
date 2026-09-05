#include <base/sections.h>
#include <kernel/boot.h>
#include <kernel/error.h>
#include <mm/mm.h>

int32_t __init mm_init(void) {

  struct boot_state *boot = boot_get_state();

  KERR_TRY(pmm_init(boot->memory_map, boot->memory_map_count));

  KERR_TRY(vmm_init());

  KERR_TRY(kheap_init());

  return KERR_OK;
}