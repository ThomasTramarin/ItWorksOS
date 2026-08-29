#include <arch/interrupts/irq.h>
#include <arch/interrupts/pic.h>
#include <drivers/irqchip/i8259.h>
#include <irq/irq.h>

void x86_irq_handler(struct x86_interrupt_frame *frame) {
  irq_dispatch(i8259_get_chip(), i8259_get_map(), frame->int_no);
}