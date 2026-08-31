# Kernel Heap Allocator (kheap)

## Overview
The kernel heap provides dynamic memory allocation for the kernel at byte-level granularity.

While PMM operates on 4 KiB frames and the VMM manages virtual memory mappings, the kernel heap allows kernel subsystems to request memory blocks of variable size.

The kernel heap depends on VMM, but can be used immediately after `vmm_init` because returned pointers resides on the kernel LowMem and are always mapped to physical memory. For this reason, even the VMM uses `kmalloc` to allocate dynamic memory for virtual memory areas and new address spaces.

## Intended Use
The heap is optimized for small and medium size kernel data structures. For large contiguous buffers or page-aligned memory, kernel subsystems should request memory directly from the PMM or VMM.

## Memory Allocator Design 

The KHEAP is implemented as an **explicit free list allocator** using **boundary tags** (header + footer) for constant-time bidirectional coalescing.

### Block
Every memory block is enclosed between a 16-byte header and a 4-byte footer. Payload is 4-byte aligned.

### Allocation Strategy
- **Search Policy**: uses a *First-Fit** policy to find available blocks in the free list
- **Block Splitting**: if the remaining space of a free block after allocation is large enough to form a valid minimum block, it is splitted into an allocated block and a new free block
- **Coalescing**: when freeing a block, the allocator inspects adjacent blocks. If neighbor blocks are free, they are merged to prevent memory fragmentation

## Current Limitations
- **Static Size**: the current implementation allocates 256 physical frames (1 MiB) during initialization and does not dynamically grow

## Initialization Phase
The `kheap_init` function is called to set up the initial heap area:
1. Requests 256 contiguous physical frames from the PMM in LowMem space
2. Converts the physical address to a virtual address
3. Constructs a single large free block 
4. Initializes the free list and inserts the inital block

## Runtime Phase
- `kmalloc(size)`: scans the free list using first-fit policy, splits the block if possible, marks it as allocated and returns the payload pointer
- `kfree(ptr)`: validates the pointer, marks the block as free, merges adjacent free blocks and updates the free list

