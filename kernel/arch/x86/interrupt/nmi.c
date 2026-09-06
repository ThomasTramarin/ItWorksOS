#include <arch/interrupts/nmi.h>
#include <log/panic.h>

void x86_nmi_handler(struct x86_interrupt_frame *frame) {
  panic("NMI received");
}