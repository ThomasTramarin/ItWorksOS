#include <base/compiler.h>
#include <base/stdarg.h>
#include <base/stdbool.h>
#include <hal/cpu.h>
#include <hal/interrupt.h>
#include <kernel/panic.h>
#include <log/printk.h>

static bool panicking = false;

void __noreturn panic(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vpanic(fmt, args);
}

void __noreturn vpanic(const char *fmt, va_list args) {
  hal_interrupt_disable();

  if (panicking) {
    goto halt;
  }
  panicking = true;

  pr_fatal("\n\n--- KERNEL PANIC ---\n");

  vprintk(LOG_FATAL, fmt, args);
  pr_fatal("\n");

  // TODO: HAL dump registers

halt:
  pr_fatal("\n--- System Halted ---\n");
  while (1) {
    hal_cpu_halt();
  }
}