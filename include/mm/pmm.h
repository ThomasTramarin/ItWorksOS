#ifndef MM_PMM
#define MM_PMM
#include <base/stdint.h>
#include <boot/boot_info.h>

void pmm_init(struct boot_mem_map_entry *map_ptr, uint16_t count);
void pmm_alloc(void);
void pmm_free(void);

#endif