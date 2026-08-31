# Virtual Memory Manager (VMM)

## Overview
The Virtual Memory Manager (VMM) is the subsystem responsible for managing virtual address spaces and memory protection.

The VMM abstracts hardware translation structures to map virtual memory addresses to physical addresses.

## Initialization Phase
The `vmm_init` function sets up the kernel address space. Because the Kernel Heap is not yet available, the initial kernel virtual memory space is allocated statically.

During initialization:
1. **Architecture Delegation**: Calls `hal_mm_space_init()` to initialize the  architecture-specific memory structures for the kernel.
2. **Kernel Address Space**: The HAL allocates the root page structure from the PMM, sets up the higher-half direct mapping for kernel LowMem, and loads the new translation table into the CPU.