#include <syscall/syscall.h>

int32_t sys_write(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4,
                  uint32_t arg5);

static syscall_fn_t syscall_table[] = {
    [SYS_WRITE] = sys_write,
};

int32_t syscall_dispatch(struct syscall_args *args) {
  if (args->number >= sizeof(syscall_table) / sizeof(syscall_table[0])) {
    return -SERR_NOSYS;
  }

  syscall_fn_t fn = syscall_table[args->number];

  return fn(args->arg1, args->arg2, args->arg3, args->arg4, args->arg5);
}