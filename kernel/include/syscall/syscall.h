#ifndef SYSCALL_SYSCALL_H
#define SYSCALL_SYSCALL_H

#include <base/stdint.h>

enum syscall_numbers {
  SYS_WRITE = 0,
};

enum syscall_errors {
  SERR_NOSYS = 1,
};

struct syscall_args {
  uint32_t number;
  uint32_t arg1;
  uint32_t arg2;
  uint32_t arg3;
  uint32_t arg4;
  uint32_t arg5;
};

typedef int32_t (*syscall_fn_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3,
                                uint32_t arg4, uint32_t arg5);

int32_t syscall_dispatch(struct syscall_args *args);

#endif