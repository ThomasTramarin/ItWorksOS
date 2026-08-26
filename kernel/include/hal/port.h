#ifndef HAL_PORT_H
#define HAL_PORT_H

/**
 * @file port.h
 * @brief Hardware port I/O abstraction layer
 *
 * Architectures without an I/O mechanism may not provide this functionality or
 * may implement it differently.
 */

#include <base/stdint.h>

uint8_t hal_port_read8(uint16_t port);
uint16_t hal_port_read16(uint16_t port);
uint32_t hal_port_read32(uint16_t port);

void hal_port_write8(uint16_t port, uint8_t value);
void hal_port_write16(uint16_t port, uint16_t value);
void hal_port_write32(uint16_t port, uint32_t value);

#endif