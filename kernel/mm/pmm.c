#include <base/align.h>
#include <base/bit.h>
#include <base/sections.h>
#include <base/stddef.h>
#include <boot/boot_info.h>
#include <kernel/error.h>
#include <klib/bitmap.h>
#include <log/printk.h>
#include <mm/pmm.h>

/**
 * The PMM uses a bitmap for tracking whether a frame (4 KiB) is free
 * (0) or reserved/allocated (1).
 * The bitmap is stored immediately after the end of the kernel and its size
 * depends on the physical RAM detected by the bootloader.
 * This implementation doesn't distinguish between reserved frames (e.g.
 * firmware regions, MMIO regions) and allocated frames (memory allocated).
 */

// from the linker
extern uint8_t __kernel_start[];
extern uint8_t __kernel_end[];

#define PMM_LOG "PMM: "

#define KERNEL_VIRT_BASE ((vaddr_t)0xC0000000)

static struct {
  struct bitmap bm;

  /* End of Physical address range in memory map */
  paddr_t max_addr;

  /* Number of frames represented by the bitmap */
  uint32_t total_frames;

  /* Number of currently allocatable frame */
  uint32_t free_frames;
} pmm;

/* Convert a physical address to a frame number */
static inline uint32_t pmm_addr_to_frame(paddr_t addr) {
  return (uint32_t)(addr / PMM_FRAME_SIZE);
}

/* Convert a frame number to a physical address */
static inline paddr_t pmm_frame_to_addr(uint32_t frame) {
  return (paddr_t)frame * PMM_FRAME_SIZE;
}

/*
 * Check whether a frame range belongs to the PMM.
 *
 * [first_frame, first_frame + pages)
 */
static int32_t pmm_validate_range(uint32_t first_frame, size_t pages) {
  if (pages == 0)
    return -KERR_INVAL;
  if (first_frame >= pmm.total_frames)
    return -KERR_INVAL;

  if (pages > pmm.total_frames - first_frame)
    return -KERR_INVAL;

  return KERR_OK;
}

static int32_t pmm_reserve(paddr_t addr, size_t pages) {
  uint32_t first_frame;

  if (pages == 0)
    return -KERR_INVAL;

  if (addr % PMM_FRAME_SIZE != 0)
    return -KERR_INVAL;

  first_frame = pmm_addr_to_frame(addr);

  if (pmm_validate_range(first_frame, pages) < 0)
    return -KERR_INVAL;

  // First check the whole range (the region must be freed)
  for (size_t i = 0; i < pages; i++) {
    if (bitmap_test(&pmm.bm, first_frame + i)) {
      return -KERR_BUSY;
    }
  }

  // Modify the bitmap
  for (size_t i = 0; i < pages; i++) {
    bitmap_set(&pmm.bm, first_frame + i);
  }

  pmm.free_frames -= pages;

  return KERR_OK;
}

static int32_t pmm_unreserve(paddr_t addr, size_t pages) {
  uint32_t first_frame;

  if (pages == 0)
    return -KERR_INVAL;

  if (addr % PMM_FRAME_SIZE != 0)
    return -KERR_INVAL;

  first_frame = pmm_addr_to_frame(addr);

  if (pmm_validate_range(first_frame, pages) < 0)
    return -KERR_INVAL;

  // First check the whole range (the region must be currently used)
  for (size_t i = 0; i < pages; i++) {
    if (!bitmap_test(&pmm.bm, first_frame + i)) {
      return -KERR_INVAL;
    }
  }

  for (size_t i = 0; i < pages; i++) {
    bitmap_clear(&pmm.bm, first_frame + i);
  }

  // Modify the bitmap
  pmm.free_frames += pages;

  return KERR_OK;
}

/*
 * Force-marks a range as reserved during initialization.
 * Unlike pmm_reserve(), this function does not care about the previous
 * state of the frames.
 */
static void pmm_force_reserve_range(uint32_t start_frame, size_t pages) {
  for (size_t i = 0; i < pages; i++) {
    uint32_t frame = start_frame + i;
    if (frame < pmm.total_frames) {
      bitmap_set(&pmm.bm, frame);
    }
  }
}

int32_t __init pmm_init(const struct boot_mem_map_entry *map_ptr,
                        uint16_t count) {
  if (!map_ptr || count == 0)
    return -KERR_INVAL;

  pmm.max_addr = 0;

  for (uint16_t i = 0; i < count; i++) {

    if (map_ptr[i].type != BOOT_TYPE_USABLE)
      continue;

    uint64_t entry_end = map_ptr[i].base + map_ptr[i].length;

    if (entry_end > pmm.max_addr)
      pmm.max_addr = (paddr_t)entry_end;
  }

  /*
   * One bitmap bit represents one 4 KiB physical frame.
   */
  pmm.total_frames =
      (uint32_t)(ALIGN_UP(pmm.max_addr, PMM_FRAME_SIZE) / PMM_FRAME_SIZE);

  // Initialize the bitmap immediately after the kernel image
  bitmap_init(&pmm.bm, (uint32_t *)__kernel_end, pmm.total_frames);

  // mark all frames as reserved by default
  for (uint32_t i = 0; i < pmm.total_frames; i++) {
    bitmap_set(&pmm.bm, i);
  }

  // Free usable memory (type 1)
  for (uint16_t i = 0; i < count; i++) {
    if (map_ptr[i].type != BOOT_TYPE_USABLE)
      continue;

    uint64_t start = ALIGN_UP(map_ptr[i].base, PMM_FRAME_SIZE);

    uint64_t end =
        ALIGN_DOWN(map_ptr[i].base + map_ptr[i].length, PMM_FRAME_SIZE);

    uint32_t start_frame = (uint32_t)(start / PMM_FRAME_SIZE);

    uint32_t end_frame = (uint32_t)(end / PMM_FRAME_SIZE);

    for (uint32_t frame = start_frame; frame < end_frame; frame++) {

      bitmap_clear(&pmm.bm, frame);
    }
  }

  // Reserve the first MiB
  uint32_t first_mib_frames = (1024 * 1024) / PMM_FRAME_SIZE;
  pmm_force_reserve_range(0, first_mib_frames);

  paddr_t kernel_start_phys =
      (paddr_t)(uintptr_t)__kernel_start - KERNEL_VIRT_BASE;

  paddr_t kernel_end_phys = (paddr_t)(uintptr_t)__kernel_end - KERNEL_VIRT_BASE;

  // Reserve the kernel image
  uint32_t kernel_start_frame =
      pmm_addr_to_frame(ALIGN_DOWN(kernel_start_phys, PMM_FRAME_SIZE));

  uint32_t kernel_end_frame =
      pmm_addr_to_frame(ALIGN_UP(kernel_end_phys, PMM_FRAME_SIZE));

  pmm_force_reserve_range(kernel_start_frame,
                          kernel_end_frame - kernel_start_frame);

  // Reserve the PMM bitmap
  size_t bitmap_size_bytes = (pmm.total_frames + 7) / 8;

  paddr_t bitmap_start_phys = kernel_end_phys;

  paddr_t bitmap_end_phys = bitmap_start_phys + bitmap_size_bytes;

  uint32_t bitmap_start_frame =
      pmm_addr_to_frame(ALIGN_DOWN(bitmap_start_phys, PMM_FRAME_SIZE));

  uint32_t bitmap_end_frame =
      pmm_addr_to_frame(ALIGN_UP(bitmap_end_phys, PMM_FRAME_SIZE));

  pmm_force_reserve_range(bitmap_start_frame,
                          bitmap_end_frame - bitmap_start_frame);

  // Calculate final number of available free frames
  pmm.free_frames = 0;

  for (uint32_t i = 0; i < pmm.total_frames; i++) {
    if (!bitmap_test(&pmm.bm, i))
      pmm.free_frames++;
  }

  // Debug info
  uint32_t physical_address_space_mib =
      (uint32_t)(pmm.max_addr / (1024 * 1024));

  uint32_t allocatable_mib =
      (uint32_t)((pmm.free_frames * PMM_FRAME_SIZE) / (1024 * 1024));

  pr_debug(PMM_LOG "Kernel start (%p), Kernel end (%p)\n", __kernel_start,
           __kernel_end);

  pr_debug(PMM_LOG "Physical Address Space: %u MiB\n",
           physical_address_space_mib);

  pr_debug(PMM_LOG "Allocatable RAM: %u MiB\n", allocatable_mib);

  pr_debug(PMM_LOG "Total frames: %u, Free frames: %u\n", pmm.total_frames,
           pmm.free_frames);

  return KERR_OK;
}

int32_t pmm_alloc(paddr_t min, paddr_t max, size_t pages,
                  enum pmm_policy policy, paddr_t *out) {

  if (!out || pages == 0)
    return -KERR_INVAL;

  uint32_t start_frame =
      (min != 0) ? pmm_addr_to_frame(ALIGN_UP(min, PMM_FRAME_SIZE)) : 0;
  uint32_t max_frame = (max != 0) ? pmm_addr_to_frame(max) : pmm.total_frames;

  if (max_frame > pmm.total_frames)
    max_frame = pmm.total_frames;

  if (start_frame < max_frame) {
    uint32_t consecutive = 0;
    for (uint32_t i = start_frame; i < max_frame; i++) {
      if (bitmap_test(&pmm.bm, i)) {
        consecutive = 0;
        continue;
      }

      if (++consecutive == pages) {
        uint32_t first_found_frame = i - pages + 1;

        // mark frames as allocated in the bitmap
        for (size_t j = 0; j < pages; j++) {
          bitmap_set(&pmm.bm, first_found_frame + j);
        }

        pmm.free_frames -= pages;

        *out = pmm_frame_to_addr(first_found_frame);
        return KERR_OK;
      }
    }
  }

  /* Fallback try */
  if (policy == PMM_POLICY_FALLBACK) {
    return pmm_alloc(0, 0, pages, PMM_POLICY_STRICT, out);
  }

  // failed to allocate memory
  return -KERR_NOMEM;
}

int32_t pmm_free(paddr_t addr, size_t pages) {
  if (pages == 0)
    return -KERR_INVAL;

  if (addr % PMM_FRAME_SIZE != 0)
    return -KERR_INVAL; // Address must be page-aligned

  uint32_t start_frame = pmm_addr_to_frame(addr);

  if (pmm_validate_range(start_frame, pages) < 0)
    return -KERR_INVAL;

  // all frames should be currently allocated
  for (uint32_t i = start_frame; i < start_frame + pages; i++) {
    if (!bitmap_test(&pmm.bm, i)) {
      return -KERR_INVAL; // double free or invalid page state
    }
  }

  // mark frames as free
  for (uint32_t i = start_frame; i < start_frame + pages; i++) {
    bitmap_clear(&pmm.bm, i);
  }

  pmm.free_frames += pages;

  return KERR_OK;
}