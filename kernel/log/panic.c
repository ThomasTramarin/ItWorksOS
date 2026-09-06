#include <base/compiler.h>
#include <base/stdarg.h>
#include <base/stdbool.h>
#include <log/panic.h>
#include <log/printk.h>

static bool panicking = false;

void __noreturn panic(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  vpanic(fmt, args);
}

void __noreturn vpanic(const char *fmt, va_list args) {
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