#include <base/stdint.h>
#include <console/console.h>

int32_t sys_write(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4,
                  uint32_t arg5) {
  (void)arg3;
  (void)arg4;
  (void)arg5;

  // TODO: check the pointer passsed by user process
  const char *buf = (const char *)arg1;
  size_t len = arg2;

  console_write(buf, len);

  return 0;
}