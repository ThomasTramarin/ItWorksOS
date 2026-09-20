# Console

## Overview

In IWOS, a **console** is an abstraction for **text-based output**.

It provides a common interface for displaying text and controlling a text cursor without depending directly on the underlying hardware.

The console sits between the kernel and the console implementation:

- The kernel uses generic `console_*` functions, such as `console_write()`.
- The console subsystem forwards these operations to the active console implementation.

The current implementation uses **VGA text mode**.

A future implementation may support multiple registered consoles.

## Console abstraction

The abstraction is represented by `struct console`.

A console contains the state and operations needed to provide a text interface.

Its state includes:

- **Name**: identifies the console
- **Dimensions**: number of columns and rows
- **Cursor position**: current position where text is written

The console provides operations for:

- writing text
- clearing the screen
- changing the cursor position
- showing or hiding the cursor

The kernel does not need to know how these operations are implemented.

## Console operations

### Writing

`console_write()` writes characters starting at the current cursor position.

Common control characters are handled by the console:

- `\n` moves to the next line
- `\r` moves to the beginning of the current line
- `\t` advances the cursor to the next tab position
- `\b` moves the cursor back

When the cursor reaches the bottom of the screen, the console **automatically scrolls** the visible contents.

The console does **not** currently keep a **history** of scrolled lines. Content that leaves the visible area is discarded.

### Cursor

The cursor position is represented by an `(x, y)` coordinate, with `(0, 0)` at the top-left corner.

`console_set_cursor_position()` changes the cursor position.

How the cursor is represented is implementation-specific and handled by the console implementation.

### Clearing

`console_clear()` clears the console and resets the cursor to `(0, 0)`.

### Cursor visibility

`console_set_cursor_visible()` controls whether the cursor is visible.

## Console registration

IWOS currently supports a single active console.

A console implementation is registered through `console_register()`.

Once registered, it becomes the active console. The kernel can then use the generic `console_*` functions without accessing the implementation directly.
