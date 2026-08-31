# Memory Management Overview

Memory management is structured into three hierarchical layers, each providing a higher level of abstraction:

- **KHEAP (Kernel Heap):** provides byte-level dynamic allocation (`kmalloc`/`kfree`) for internal kernel structures.
- **VMM (Virtual Memory Manager):** manages virtual address spaces, page tables, and memory protection.
- **PMM (Physical Memory Manager):** manages physical memory frames (4 KiB) using a bitmap tracking system.

## Memory Layout
The kernel uses a **Higher-Half** memory model (`0xC0000000`). The memory address space is split into two main operational regions:

- **LowMem (`0xC0000000 - 0xEFFFFFFF`, 768 MiB):** directly mapped to physical memory (`0x00000000 - 0x30000000`). Contains kernel executable code, PMM bitmap structures, and the Kernel Heap.
- **HighMem:** physical memory above 768 MiB and virtual memory reserved for user space processes or dynamic kernel mappings. Accessing HighMem requires explicit virtual page mapping via the VMM.

## Initialization Order
1. `pmm_init()`: parses the memory map provided by the bootloader and initializes physical frame tracking.
2. `vmm_init()`: constructs the master kernel page directory via the HAL and enables paging with LowMem identity mapping.
3. `kheap_init()`: requests initial pages from LowMem and sets up the heap for `kmalloc` and `kfree`.