#include <base/sections.h>
#include <kernel/error.h>
#include <mm/mm.h>

int32_t __init mm_init(const struct boot_mem_map_entry *map, uint16_t count) {
  KERR_TRY(pmm_init(map, count));

  KERR_TRY(vmm_init());

  KERR_TRY(kheap_init());

  return KERR_OK;
}