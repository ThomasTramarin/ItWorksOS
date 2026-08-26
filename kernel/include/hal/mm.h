#ifndef HAL_MM_H
#define HAL_MM_H

#include <base/stdbool.h>
#include <base/stdint.h>

struct hal_vm_space;

/**
 * @brief Initialize a virtual space
 * @param space_out    The initialized hal virtual kernel space
 * @param kernel_space Pointer to the hal virtual kernel space.
 *                     If NULL, the function initializes a kernel virtual space
 *                     and returns it in space_out.
 *                     If != NULL, the function initializes a user-space virtual
 *                     space and copies higher-half kernel mappings into the
 *                     user space virtual space.
 * @return KERR_OK on success or a negative kernel error code.
 */
int32_t hal_mm_space_init(struct hal_vm_space *space_out,
                          struct hal_vm_space *kernel_space);

#endif