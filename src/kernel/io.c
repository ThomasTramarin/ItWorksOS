#include "io.h"

#define UNUSED_PORT 0x80

uint8_t inb(uint16_t port) {
  uint8_t res;
  asm volatile("inb %1, %0" : "=a"(res) : "Nd"(port));
  return res;
}

void outb(uint16_t port, uint8_t value) {
  asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void io_wait(void) { outb(UNUSED_PORT, 0); }