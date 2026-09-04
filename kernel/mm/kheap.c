#include <base/align.h>
#include <base/bit.h>
#include <base/sections.h>
#include <base/stddef.h>
#include <base/stdint.h>
#include <kernel/error.h>
#include <kernel/printk.h>
#include <klib/list.h>
#include <mm/layout.h>
#include <mm/pmm.h>

/**
 * @brief Kernel Heap Implementation
 *
 * This memory allocator is suitable for small/medium kernel allocations.
 * The heap lives within the LowMem area, so returned pointers
 * are always mapped into the kernel LowMem virtual space.
 *
 * The allocated memory is both physically and virtually contiguous.
 *
 * The implementation used an explicit free list.
 *
 * Block structure:
 *
 *   +-------------------------+
 *   | Header (16 bytes)       |
 *   +-------------------------|
 *   | Payload + Padding       |
 *   | (4-byte aligned)        |
 *   +-------------------------|
 *   | Footer (4 bytes)        |
 *   +-------------------------|
 *
 * Current implementation limitations:
 *  - The heap is limited to the available LowMem physical address space
 *  - Heap growth requires physically contiguous pages
 *
 * Due to these limitations, the current heap allocates 256 physical frames
 * (1 MiB), which is more than sufficient for the current kernel stage.
 * Heap growth is not currently supported.
 */

#define KHEAP_PADDING_ALIGN 4
#define KHEAP_LOG "KHEAP: "

enum kheap_flags {
  KHEAP_FLAG_ALLOC = BIT(0), /* 1 = allocated, 0 = free */
};

struct kheap_header {
  size_t size; /* The size of the entire block (header, payload + padding,
                  footer) */
  uint32_t flags;
  struct list_node free_node;
};

struct kheap_footer {
  size_t size; /* The size of the entire block
                  (header, payload + padding,footer) */
};

#define KHEAP_MIN_BLOCK_SIZE                                                   \
  (sizeof(struct kheap_header) + KHEAP_PADDING_ALIGN +                         \
   sizeof(struct kheap_footer))

static struct list kheap_free_list;

static vaddr_t kheap_start;
static vaddr_t kheap_end;

int32_t __init kheap_init(void) {
  paddr_t phys;

  /* Try to allocate 1 MiB */
  KERR_TRY(PMM_ALLOC_LOWMEM(256, &phys));

  kheap_start = PHYS_TO_VIRT(phys);

  /* Create the first free block */
  size_t block_size = PMM_FRAME_SIZE * 256;

  kheap_end = kheap_start + block_size;

  struct kheap_header *hdr = (struct kheap_header *)kheap_start;

  hdr->size = block_size;
  hdr->flags = 0;

  struct kheap_footer *footer =
      (struct kheap_footer *)((uint8_t *)kheap_end -
                              sizeof(struct kheap_footer));

  footer->size = block_size;

  /* Initialize free list */
  list_init(&kheap_free_list);
  list_insert_front(&kheap_free_list, &hdr->free_node);

  /* Debug logging */
  pr_debug(KHEAP_LOG "heap_start (%p), heap_end (%p)\n", kheap_start,
           kheap_end);

  return KERR_OK;
}

void *kmalloc(size_t size) {

  if (size == 0)
    return KERR_PTR(-KERR_INVAL);

  size_t payload_size = ALIGN_UP(size, KHEAP_PADDING_ALIGN);

  size_t block_size =
      sizeof(struct kheap_header) + payload_size + sizeof(struct kheap_footer);

  struct list_node *node;

  /* First-fit free list */
  list_for_each(node, &kheap_free_list) {

    struct kheap_header *hdr = (struct kheap_header *)container_of(
        node, struct kheap_header, free_node);

    if (hdr->size >= block_size) {
      /*
       * Block found
       *
       * 2 cases:
       *  - do not split if the remained space is not sufficient to allocate
       *    even the minimum-size block
       *  - split if the free block remained after the allocation still can
       *    contain another block with minimum requirements
       *    (header size + payload min + footer size)
       */

      size_t old_size = hdr->size;
      size_t remaining = old_size - block_size;

      if (remaining >= KHEAP_MIN_BLOCK_SIZE) {
        // split is possible

        // update the header
        MASK_SET(hdr->flags, KHEAP_FLAG_ALLOC);
        hdr->size = block_size;

        // create a new footer
        struct kheap_footer *footer =
            (struct kheap_footer *)((uint8_t *)hdr + block_size -
                                    sizeof(struct kheap_footer));

        footer->size = block_size;

        // the new free block starts exactly after the allocated block
        struct kheap_header *free_hdr =
            (struct kheap_header *)(struct kheap_footer *)&footer[1];

        MASK_CLEAR(free_hdr->flags, KHEAP_FLAG_ALLOC); // mark the block as free
        free_hdr->size = remaining;                    // update free block size

        // create the new footer
        struct kheap_footer *free_footer =
            (struct kheap_footer *)((uint8_t *)free_hdr + remaining -
                                    sizeof(struct kheap_footer));

        free_footer->size = remaining;

        // replace the old free list header with the new one
        list_replace(&hdr->free_node, &free_hdr->free_node);

      } else {
        // split is not possible

        // remove the block from the freelist
        list_remove(&hdr->free_node);

        // mark the entire block as allocated
        MASK_SET(hdr->flags, KHEAP_FLAG_ALLOC);
      }

      // return a valid pointer
      return (void *)((struct kheap_header *)&hdr[1]);
    }
  }

  return KERR_PTR(-KERR_NOMEM);
}

/**
 * @brief Merge two adjacent free blocks
 *
 * @param left Pointer to the first header
 * @param right Pointer to the second header
 *
 * This function does not update the freelist and does not check for errors:
 *   - both blocks must be free
 *   - left must be before right in memory
 *   - left and right blocks must be adjacent
 */
static void kheap_merge(struct kheap_header *left, struct kheap_header *right) {
  left->size += right->size;

  struct kheap_footer *footer =
      (struct kheap_footer *)((uint8_t *)right + right->size -
                              sizeof(struct kheap_footer));

  footer->size = left->size;
}

int32_t kfree(void *ptr) {
  if (!ptr)
    return -KERR_INVAL;

  if ((vaddr_t)ptr < kheap_start + sizeof(struct kheap_header) ||
      (vaddr_t)ptr >= (kheap_end - KHEAP_MIN_BLOCK_SIZE))
    return -KERR_INVAL;

  // the pointer should also be 4-byte aligned
  if ((vaddr_t)ptr % 4 != 0)
    return -KERR_INVAL;

  struct kheap_header *hdr =
      (struct kheap_header *)((uint8_t *)ptr - sizeof(struct kheap_header));

  // mark the block as free
  if (!MASK_TEST(hdr->flags, KHEAP_FLAG_ALLOC))
    return -KERR_INVAL;

  MASK_CLEAR(hdr->flags, KHEAP_FLAG_ALLOC);

  /**
   * Coalescing
   *
   * After freeing the current block, check whether the previous and next blocks
   * are free to make a larger and unique contiguous free block.
   */
  struct kheap_header *next_hdr = NULL;
  struct kheap_footer *prev_footer = NULL;
  struct kheap_header *prev_hdr = NULL;

  bool next_free = false;
  bool prev_free = false;

  /* Check whether the next block is free
   * If ptr points to the last heap block, do not access the next header
   * (it does not exist)
   */
  if ((vaddr_t)hdr + hdr->size < kheap_end) {
    next_hdr = (struct kheap_header *)((uint8_t *)hdr + hdr->size);

    next_free = !MASK_TEST(next_hdr->flags, KHEAP_FLAG_ALLOC);
  }

  /* Check whether the previous block is free
   * If ptr points to the first heap block, do not dereference the block
   * size stored on the footer
   */
  if ((vaddr_t)hdr != kheap_start) {
    prev_footer =
        (struct kheap_footer *)((uint8_t *)hdr - sizeof(struct kheap_footer));

    prev_hdr = (struct kheap_header *)((uint8_t *)hdr - prev_footer->size);

    prev_free = !MASK_TEST(prev_hdr->flags, KHEAP_FLAG_ALLOC);
  }

  /* Coalescing Logic
   *
   * Merge the current block with adjacent free blocks.
   *
   * If the previous block is free, merge it with the current block and use the
   * previous block as the resulting block.
   *
   * If the next block is free, remove it from the free list and merge it with
   * the resulting block
   *
   * The resulting block is inserted into the free list only if the previous
   * block was not already free
   */
  if (prev_free) {
    kheap_merge(prev_hdr, hdr);
    hdr = prev_hdr;
  }

  if (next_free) {
    list_remove(&next_hdr->free_node);
    kheap_merge(hdr, next_hdr);
  }

  if (!prev_free) {
    list_insert_front(&kheap_free_list, &hdr->free_node);
  }

  return KERR_OK;
}

void kheap_dump(void) {

  vaddr_t addr = kheap_start;

  size_t total_blocks = 0;
  size_t allocated_blocks = 0;
  size_t free_blocks = 0;
  size_t allocated_bytes = 0;
  size_t free_bytes = 0;

  printk("KHEAP DUMP\n");
  printk("--------------------------------\n");

  while (addr < kheap_end) {
    struct kheap_header *hdr = (struct kheap_header *)addr;

    bool allocated = MASK_TEST(hdr->flags, KHEAP_FLAG_ALLOC);

    printk("%p   %d   %s\n", hdr, hdr->size, allocated ? "ALLOC" : "FREE");

    total_blocks++;

    if (allocated) {
      allocated_blocks++;
      allocated_bytes += hdr->size;
    } else {
      free_blocks++;
      free_bytes += hdr->size;
    }

    addr += hdr->size;
  }

  printk("blocks:     %d\n", total_blocks);
  printk("allocated:  %d (%d bytes)\n", allocated_blocks, allocated_bytes);
  printk("free:       %d (%d bytes)\n", free_blocks, free_bytes);

  printk("FREELIST\n");
  struct list_node *node;
  list_for_each(node, &kheap_free_list) {
    struct kheap_header *hdr =
        container_of(node, struct kheap_header, free_node);

    printk("%p   %d\n", hdr, hdr->size);
  }
  printk("--------------------------------\n");
}