#ifndef IO_H
#define IO_H
#include <base/stdint.h>

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void io_wait(void);

#endif