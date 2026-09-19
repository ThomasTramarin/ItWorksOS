#ifndef BASE_SECTIONS_H
#define BASE_SECTIONS_H

#include <base/compiler.h>

/**
 * @brief Place a function in the kernel initialization section
 *
 * Function places in this section are only required during kernel
 * initialization and not used during normal kernel execution.
 * Their memory may be reclaimed after initialization completes.
 *
 * Example:
 *      static int __init func(void);
 */
#define __init __section(".init.text")

/**
 * @brief Place data in the kernel initialization section
 *
 * Data placed in this section is only required during kernel initialization and
 * is not used during normal kernel execution.
 * Its memory may be reclaimed after initialization completes.
 *
 * Example:
 *      static int __init_data var = 23;
 */
#define __init_data __section(".init.data")

extern char __kernel_start[];
extern char __kernel_end[];

extern char __init_text_start[];
extern char __init_text_end[];

extern char __text_start[];
extern char __text_end[];

extern char __data_start[];
extern char __data_end[];

extern char __rodata_start[];
extern char __rodata_end[];

extern char __initcall_devdrv_start[];
extern char __initcall_devdrv_end[];

extern char __init_bss_start[];
extern char __init_bss_end[];

extern char __bss_start[];
extern char __bss_end[];

#endif