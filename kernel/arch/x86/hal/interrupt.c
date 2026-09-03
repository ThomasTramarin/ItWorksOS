#include <arch/cpu/flags.h>
#include <base/bit.h>
#include <hal/interrupt.h>

void hal_interrupt_enable(void) { x86_sti(); }

void hal_interrupt_disable(void) { x86_cli(); }

bool hal_interrupt_enabled(void) { return x86_interrupts_enabled(); }

bool hal_interrupt_save_disable(void) { return x86_interrupt_save_disable(); }

void hal_interrupt_restore(bool state) { x86_interrupt_restore(state); }