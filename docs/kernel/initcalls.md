# Initcalls

## Overview

An **initcall** is a function that the kernel automatically executes during initialization.

Initcalls allow kernel components to register initialization functions without maintaining a central list of functions.

An initcall is registered with the `INITCALL()` macro.

For example:

```c
static int32_t __init timer_init(void)
{
    ...
}

INITCALL(INIT_DEVDRV, timer_init);
```

The macro creates a `struct initcall` containing the function pointer and its name, and places it in an ELF section associated with the selected initcall level.

## Initcall levels

Initcalls are grouped into **levels**.

Each level represents a group of initialization functions that the kernel can invoke separately.

The current implementation provides the `devdrv` level:

```c
INITCALL(INIT_DEVDRV, timer_init);
```

This places the initcall in the logical section:

```text
.initcall.devdrv
```

The kernel invokes this level through:

```c
initcalls_invoke_devdrv();
```

Additional levels can be added when different initialization stages are required.

## Registration

`INITCALL()` creates a `struct initcall` entry:

```c
struct initcall {
    const char *name;
    init_fn_t fn;
};
```

The entry stores the function name for logging and a pointer to the initialization function.

The entry is placed in the ELF section corresponding to the selected level.

This allows different source files to register initcalls independently.

## Invocation

The linker collects all initcalls belonging to the same level into a contiguous section.

The linker script defines symbols marking the beginning and end of the collected entries.

The kernel uses these symbols as the bounds of the initcall range and iterates over the entries.

Each entry is invoked in sequence through its function pointer.

If an initcall returns a negative error code, invocation stops and the error is returned to the caller. The kernel may then treat the failure as fatal.

A successful initcall allows the kernel to continue with the next entry.

## Initialization-only code

An initcall function can be marked with `__init` when its code is only needed during initialization:

```c
static int32_t __init timer_init(void)
{
    ...
}
```

Such functions are placed in `.init.text` (see [ELF sections](./elf_sections.md)).
