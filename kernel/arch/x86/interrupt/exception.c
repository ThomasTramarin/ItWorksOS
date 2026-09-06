#include <arch/interrupts/exception.h>
#include <kernel/exception.h>
/**
 * @brief Translate from vector interrupt number to portable exception type
 */
static enum exception_type x86_exception_translate(uint32_t vector) {
  switch (vector) {
  case 0:
    return EXCEPTION_ARITHMETIC;
  default:
    return EXCEPTION_UNKNOWN;
  }
}

void x86_exception_handler(struct x86_interrupt_frame *frame) {

  // build portable exception_info
  struct exception_info info = {
      .type = x86_exception_translate(frame->int_no),
      .ip = frame->eip,
      .user_mode =
          ((frame->cs & 0x3) == 3), // read CPL (Current Privilege Level)
      .arch_ctx = frame,
  };

  exception_dispatch(&info);
}