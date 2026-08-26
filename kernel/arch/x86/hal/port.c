#include <arch/io.h>
#include <hal/port.h>

/**
 * @file port.c
 *
 * @brief x86 Port I/O HAL implementation
 */

uint8_t hal_port_read8(uint16_t port) { return x86_inb(port); }
uint16_t hal_port_read16(uint16_t port) { return x86_inw(port); }
uint32_t hal_port_read32(uint16_t port) { return x86_inl(port); }

void hal_port_write8(uint16_t port, uint8_t value) { x86_outb(port, value); }
void hal_port_write16(uint16_t port, uint16_t value) { x86_outw(port, value); }
void hal_port_write32(uint16_t port, uint32_t value) { x86_outl(port, value); }
