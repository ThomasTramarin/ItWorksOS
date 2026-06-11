#include "io.h"

#define UNUSED_PORT 0x80

unsigned char inb(unsigned short port) {
  unsigned char res;
  asm volatile("inb %1, %0" : "=a"(res) : "Nd"(port));
  return res;
}

void outb(unsigned short port, unsigned char value) {
  asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void io_wait(void) { outb(UNUSED_PORT, 0); }