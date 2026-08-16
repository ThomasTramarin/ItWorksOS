#ifndef KERNEL_KMAIN_H
#define KERNEL_KMAIN_H

#include <boot/boot_info.h>

void kmain(uint32_t magic, paddr_t boot_info);

#endif