#include <arch/interrupts/nmi.h>
#include <kernel/panic.h>

void x86_nmi_handler(struct x86_interrupt_frame *frame) {
  panic("NMI received");
}