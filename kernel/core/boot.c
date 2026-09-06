#include <base/sections.h>
#include <boot/boot_info.h>
#include <kernel/boot.h>
#include <klib/memory.h>
#include <log/printk.h>
#include <mm/layout.h>

static struct boot_state kernel_boot_state;
static struct boot_mem_map_entry kernel_boot_mmap_entries[BOOT_MEMORY_MAP_MAX];

#define BOOT_LOG "Boot: "

void __init boot_init(struct boot_info *info) {

  /*
   * Copy stage2 memory map into kernel memory.
   * PHYS_TO_VIRT can be used because of the initial 4MiB identity mapping
   */
  memcpy(kernel_boot_mmap_entries,
         (struct boot_mem_map_entry *)PHYS_TO_VIRT(info->memory_map_phys),
         sizeof(struct boot_mem_map_entry) * BOOT_MEMORY_MAP_MAX);

  kernel_boot_state.memory_map = kernel_boot_mmap_entries;
  kernel_boot_state.memory_map_count = info->memory_map_count;

  memcpy(&kernel_boot_state.video, &info->video, sizeof(struct boot_video));

  /*
   * Print boot state information
   */

  pr_debug(BOOT_LOG "Physical Memory Map:\n");

  for (uint16_t i = 0; i < kernel_boot_state.memory_map_count; i++) {
    pr_debug(BOOT_LOG "[%d] base=%x len=%x type=%x attr=%x\n", i,
             (uint32_t)kernel_boot_state.memory_map[i].base,
             (uint32_t)kernel_boot_state.memory_map[i].length,
             kernel_boot_state.memory_map[i].type,
             kernel_boot_state.memory_map[i].attr);
  }

  if (kernel_boot_state.video.type == BOOT_VIDEO_TYPE_TEXT) {
    pr_debug(
        BOOT_LOG "Video: type=TEXT mode=0x%x buf=%p rows=%d cols=%d page=%u\n",
        kernel_boot_state.video.text.mode,
        (void *)(uintptr_t)kernel_boot_state.video.text.buffer,
        kernel_boot_state.video.text.rows, kernel_boot_state.video.text.cols,
        kernel_boot_state.video.text.page);
  }
}

struct boot_state *boot_get_state(void) { return &kernel_boot_state; }