#include <hal/io.h>

/* x86 supports Port IO */

uint8_t hal_io_inb(uint16_t port) {
  uint8_t value;
  __asm__ __volatile__("inb %1, %0"
                       : "=a"(value) /* %0: output 'value' in AL */
                       : "Nd"(port)  /* %1: input 'port' in DX (or imm8) */
  );
  return value;
}

uint16_t hal_io_inw(uint16_t port) {
  uint16_t value;
  __asm__ __volatile__("inw %1, %0"
                       : "=a"(value) /* %0: output 'value' in AX */
                       : "Nd"(port)  /* %1: input 'port' in DX (or imm8) */
  );
  return value;
}

uint32_t hal_io_inl(uint16_t port) {
  uint32_t value;
  __asm__ __volatile__("inl %1, %0"
                       : "=a"(value) /* %0: output 'value' in EAX */
                       : "Nd"(port)  /* %1: input 'port' in DX (or imm8) */
  );
  return value;
}

void hal_io_outb(uint16_t port, uint8_t value) {
  __asm__ __volatile__("outb %0, %1"
                       :             /* no output */
                       : "a"(value), /* %0: input 'value' in AL */
                         "Nd"(port)  /* %1: input 'port' in DX (or imm8) */
  );
}

void hal_io_outw(uint16_t port, uint16_t value) {
  __asm__ __volatile__("outw %0, %1"
                       :             /* no output */
                       : "a"(value), /* %0: input 'value' in AX */
                         "Nd"(port)  /* %1: input 'port' in DX (or imm8) */
  );
}

void hal_io_outl(uint16_t port, uint32_t value) {
  __asm__ __volatile__("outl %0, %1"
                       :             /* no output */
                       : "a"(value), /* %0: input 'value' in EAX */
                         "Nd"(port)  /* %1: input 'port' in DX (or imm8) */
  );
}