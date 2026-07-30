#ifndef HAL_IO_H
#define HAL_IO_H

#include <base/stdint.h>

/* Port Input Output */

uint8_t hal_io_inb(uint16_t port);
uint16_t hal_io_inw(uint16_t port);
uint32_t hal_io_inl(uint16_t port);

void hal_io_outb(uint16_t port, uint8_t value);
void hal_io_outw(uint16_t port, uint16_t value);
void hal_io_outl(uint16_t port, uint32_t value);

#endif