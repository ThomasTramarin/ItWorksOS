#ifndef MM_VMM_H
#define MM_VMM_H

#include <base/stdint.h>

/**
 * @brief Initialize the virtual memory manager
 *
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t vmm_init(void);

#endif