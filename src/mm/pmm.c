#include "base/align.h"
#include <base/bit.h>
#include <base/stddef.h>
#include <boot/boot_info.h>
#include <kernel/printk.h>
#include <klib/bitmap.h>
#include <klib/memory.h>
#include <mm/pmm.h>

/**
 * The PMM uses a bitmap for tracking whether a frame (4 KiB) is free
 * (0) or used (1).
 * The bitmap is stored immediately after the end of the kernel and its size
 * depends on the physical RAM detected by the bootloader.
 */

// from the linker
extern void kernel_start;
extern void kernel_end;

#define PMM_PAGE_SIZE 4096
#define PMM_LOG "PMM: "

static struct {
  struct bitmap bm;
  uintptr_t max_phys_addr;
  uint32_t total_frames;
  uint32_t free_frames;
} pmm;

/**
 * @brief Initialize Physical Memory Manager
 */
void pmm_init(struct boot_mem_map_entry *map_ptr, uint16_t count) {

  memset(&pmm, 0, sizeof(pmm));

  // find the greatest physical address
  for (uint16_t i = 0; i < count; i++) {
    uintptr_t entry_end = map_ptr[i].base + map_ptr[i].length;
    if (entry_end > pmm.max_phys_addr)
      pmm.max_phys_addr = entry_end;
  }

  pmm.total_frames = pmm.max_phys_addr / PMM_PAGE_SIZE;

  bitmap_init(&pmm.bm, (uint32_t *)&kernel_end, pmm.total_frames);

  // mark all frames as used
  for (uint32_t i = 0; i < pmm.total_frames; i++) {
    bitmap_set(&pmm.bm, i);
  }

  // free usable memory (type 1)
  for (uint16_t i = 0; i < count; i++) {
    if (map_ptr[i].type == BOOT_TYPE_USABLE) {

      uint64_t start_frame =
          ALIGN_UP(map_ptr[i].base, PMM_PAGE_SIZE) / PMM_PAGE_SIZE;
      uint64_t end_frame =
          ALIGN_DOWN(map_ptr[i].base + map_ptr[i].length, PMM_PAGE_SIZE) /
          PMM_PAGE_SIZE;

      for (uint64_t j = start_frame; j < end_frame; j++) {
        bitmap_clear(&pmm.bm, j);
      }
    }
  }

  /*
   * Protect critical area (the first MiB)
   * The kernel and the bitmap are stored in the first MiB
   */
  for (uint64_t i = 0; i < (1024 * 1024) / PMM_PAGE_SIZE; i++) {
    bitmap_set(&pmm.bm, i);
  }

  // calculate free frames
  for (uint32_t i = 0; i < pmm.total_frames; i++) {
    if (bitmap_test(&pmm.bm, i) == 0)
      pmm.free_frames++;
  }

  pr_debug(PMM_LOG "Kernel start (%p), Kernel end (%p)\n", &kernel_start,
           &kernel_end);

  pr_debug(PMM_LOG "RAM detected (%dMiB)\n", (pmm.max_phys_addr / 1024 / 1024));
  pr_debug(PMM_LOG "Total frames (%d), Free frames: (%d)\n", pmm.total_frames,
           pmm.free_frames);
}

uintptr_t pmm_alloc_frame(void) {
  size_t bit;
  if (!bitmap_find_zero(&pmm.bm, &bit)) {
    return 0; // invalid phisical address
  }
  bitmap_set(&pmm.bm, bit);
  pmm.free_frames--;

  return bit * PMM_PAGE_SIZE;
}

void pmm_free_frame(uintptr_t addr) {
  size_t frame = addr / PMM_PAGE_SIZE;

  if (frame >= pmm.total_frames)
    return;

  if (!bitmap_test(&pmm.bm, frame))
    return; // double free

  bitmap_clear(&pmm.bm, frame);
  pmm.free_frames++;
}