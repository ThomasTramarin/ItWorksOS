# Physical Memory Manager (PMM)

## Overview
The Physical Memory Manager (PMM) is the first layer of the memory management subsystem.

The PMM divides the physical memory space into fixed-size 4 KiB regions called frames and maintains a data structure to record which frames are free or allocated/reserved.

The PMM only works with physical addresses, virtual memory is managed by the VMM (Virtual Memory Manager).

The bootloader collects information about physical memory regions using firmware interrupts, so the PMM can determine how many frames to track.

The PMM provides low-level allocations of single or multiple physically contiguous frames to higher layers.

## Bitmap Data Structure

The PMM uses a bitmap data structure to track frame state. Each bit represents exactly one 4 KiB physical frame:
- `0`: Frame is **Free** and available for allocation
- `1`: Frame is **Reserved or Allocated**

For example:
- The bit 0 determines whether the first frame (0x0000 - 0x0FFF) is allocated or free
- The bit 1 determines whether the second frame (0x1000 - 0x1FFF) is allocated or free

### Benefits
- This data structure consumes minimal space for tracking (e.g. 1 GiB of physical memory requires only 32 KiB).

### Drawbacks
- Only the current state of frames is tracked, not information about each allocation. The caller is responsible for tracking the returned address and the number of allocated pages.
- With only one bit, it is not possible to determine if a frame is reserved (e.g. MMIO device space) or just allocated. Trying to free a MMIO area may return any error.

## Initialization Phase
The `pmm_init` function must be called before any allocation.
This function:
1. Parses physical RAM layout provided by the bootloader
2. Marks the entire space as reserved
3. Marks usable regions as free
4. Reserves critical space that must never be overwritten:  
    - Early firmware / BIOS memory regions  
    - The kernel binary image  
    - The PMM bitmap itself (placed in physical memory immediately after the kernel image)  

## Runtime Phase
Once initialized, the PMM serves incoming allocation requests:
- **Frame Allocation**: the PMM scans the bitmap for available contiguous frames, marks them as reserved and returns the physical address.
- **Frame Release**: the PMM marks frames as free in the bitmap, making them available for future requests.