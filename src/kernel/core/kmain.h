#ifndef KERNEL_H
#define KERNEL_H

#include <boot/boot_info.h>

void kmain(uint32_t magic, boot_info_t *info);

#endif