#ifndef ARCH_X86_INTERRUPTS_PIC_H
#define ARCH_X86_INTERRUPTS_PIC_H

#include <base/stdint.h>

/**
 * @brief PIC initialization
 *
 * PICs IRQ remapping:
 * - Master interrupts: 32 - 39
 * - Slave interrupts: 40 - 47
 *
 * All IRQs are disabled
 */
void x86_pic_init(void);

/**
 * @brief Disable an IRQ line
 * @param irq range 0 - 15
 */
void x86_pic_mask_irq(uint8_t irq);

/**
 * @brief Enable an IRQ line
 * @param irq range 0 - 15
 */
void x86_pic_unmask_irq(uint8_t irq);

/**
 * @brief Sends End Of Interrupt (EOI) to the PIC
 *
 * @param irq range 0 - 15
 */
void x86_pic_send_eoi(uint8_t irq);

/**
 * @brief Reads the In-Service Register (ISR) of Master and Slave
 *
 * The ISR indicates which IRQs are currently being serviced
 *
 * @return uint16_t Master ISR (lower byte), Slave ISR (higher byte)
 */
uint16_t x86_pic_read_isr(void);

/**
 * @brief Reads the Interrupt Request Register (IRR) of Master and Slave
 *
 * The IRR indicates which IRQs are pending service
 *
 * @return uint16_t Master IRR (lower byte), Slave IRR (higher byte)
 */
uint16_t x86_pic_read_irr(void);

#endif