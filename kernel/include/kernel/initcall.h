#ifndef KERNEL_INITCALL_H
#define KERNEL_INITCALL_H

#include <base/compiler.h>
#include <base/stdint.h>

typedef int32_t (*init_fn_t)(void);

struct initcall {
  const char *name; /* Function name (logging) */
  init_fn_t fn;     /* Function pointer */
};

#define INIT_DEVDRV "devdrv"

/**
 * @brief Put an initialization function inside .initcalls ELF section
 *
 * The kernel will invoke init functions one by one.
 *
 * Usage Example:
 *    int32_t my_driver_init(void) {
 *      ...
 *    }
 *
 *    INITCALL(INIT_DEVDRV, my_driver_init)
 */
#define INITCALL(level, func)                                                  \
  static struct initcall __initcall_##func __used __section(                   \
      ".initcall." level) = {.name = #func, .fn = func}

/**
 * @brief Invoke all initcalls of a specified level
 *
 * @param level The INIT_* string macro (e.g. INIT_DEVDRV)
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t initcalls_invoke_devdrv(void);

#endif