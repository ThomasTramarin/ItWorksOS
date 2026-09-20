# ELF Sections

## Overview

The kernel image is divided into different **ELF sections**.

Each section contains a specific type of code or data. Keeping them separate allows the kernel to organize its memory layout and treat initialization-only content differently from permanent kernel content.

IWOS currently uses the following sections.

## Kernel sections

### `.init.text`

Contains code that is only needed during kernel initialization.

Functions marked with `__init` are placed in this section.

This code can be separated from the permanent kernel code and may be reclaimed after initialization in the future.

### `.text`

Contains the normal kernel code that remains available after initialization.

### `.data`

Contains initialized writable kernel data.

### `.rodata`

Contains read-only kernel data, such as constant data and strings.

### `.bss`

Contains uninitialized kernel data.

The section does not store the initial zero values in the kernel image. The memory is initialized to zero when the kernel starts.

## Initialization data

### `.init.bss`

Contains uninitialized data used only during kernel initialization.

Like `.bss`, the memory is initialized to zero, but it is kept separate because it belongs to the initialization phase.

This section may be reclaimed after initialization in the future.

### `.initcalls`

Contains `struct initcall` entries used by the kernel during startup.

The section is populated by more specific initcall sections such as `.initcall.devdrv`.

## Memory layout

The linker script places these sections in the kernel's virtual address space and aligns them to page boundaries.

The general layout is:

```text
.init.text
.text
.data
.rodata
.initcalls
.init.bss
.bss
```

The exact addresses are determined by the linker script.

The separation between sections allows the kernel to distinguish permanent memory from data and code used only during initialization.
