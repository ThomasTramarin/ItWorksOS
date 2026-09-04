#include <base/sections.h>
#include <base/stddef.h>
#include <kernel/error.h>
#include <kernel/initcall.h>
#include <kernel/printk.h>
#include <klib/string.h>

extern struct initcall __initcall_devdrv_start;
extern struct initcall __initcall_devdrv_end;

static __init int32_t initcalls_invoke_range(struct initcall *start,
                                             struct initcall *end) {

  for (struct initcall *ic = start; ic < end; ic++) {
    int32_t ret = ic->fn();

    if (ret < 0) {
      pr_err("initcall: %s failed\n", ic->name);
      return ret;
    }

    pr_debug("initcall: %s succeed\n", ic->name);
  }

  return KERR_OK;
}

int32_t __init initcalls_invoke_devdrv(void) {
  return initcalls_invoke_range(&__initcall_devdrv_start,
                                &__initcall_devdrv_end);
}
