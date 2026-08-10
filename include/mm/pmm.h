#ifndef MM_PMM
#define MM_PMM
#include <base/stdint.h>
#include <boot/boot_info.h>

void pmm_init(struct boot_mem_map_entry *map_ptr, uint16_t count);
uintptr_t pmm_alloc_frame(void);
void pmm_free_frame(uintptr_t addr);

#endif