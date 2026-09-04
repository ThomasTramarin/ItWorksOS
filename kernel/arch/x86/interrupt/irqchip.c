#include <base/sections.h>
#include <drivers/irqchip/i8259.h>
#include <kernel/arch.h>
#include <kernel/error.h>

int32_t __init arch_irqchip_init(void) {

  // Currently, x86 only supports PIC (Programmabe Interrupt Controller)
  KERR_TRY(i8259_init());

  return KERR_OK;
}