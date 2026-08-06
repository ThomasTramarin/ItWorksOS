#include <arch/x86/interrupts/irq.h>
#include <arch/x86/interrupts/pic.h>

void x86_irq_handler(struct x86_interrupt_frame *frame) {
  /*
   * Currently, only PIC is available
   * Vector mapping:
   *    IRQ0-7 -> 32-39
   *    IRQ8-15 -> 40-47
   */
  uint32_t irq = frame->int_no - 32;

  // Total number of IRQs (with master and slave PICs)
  if (irq >= 16)
    return;

  if (x86_pic_is_spurious(irq)) {
    x86_pic_handle_spurious(irq);
    return;
  }

  // TODO: call kernel irq_handler

  // if the IRQ is not spurious, send the EOI to the PIC(s)
  x86_pic_send_eoi(irq);
}