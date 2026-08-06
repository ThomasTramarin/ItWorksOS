#include <hal/cpu.h>

void hal_cpu_halt(void) { __asm__ __volatile__("hlt" ::: "memory"); }