#include <base/stdarg.h>
#include <base/stdbool.h>
#include <kernel/panic.h>
#include <kernel/printk.h>

static bool panicking = false;

void panic(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  vpanic(fmt, args);

  va_end(args);
}

void vpanic(const char *fmt, va_list args) {
  // TODO: call HAL functions

  // disable interrupts
  asm volatile("cli");

  if (panicking) {
    goto halt;
  }
  panicking = true;

  pr_fatal("\n\n--- KERNEL PANIC ---\n");

  vprintk(fmt, args);
  printk("\n");

  // TODO: HAL dump registers

halt:
  pr_fatal("\n--- System Halted ---\n");
  while (1) {
    asm volatile("hlt");
  }
}