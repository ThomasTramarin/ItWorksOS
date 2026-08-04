#include <kernel/exception.h>
#include <kernel/panic.h>
#include <kernel/printk.h>

void exception_dispatch(struct exception_info *info) {
  panic("Exception %d at %p", info->type, info->ip);
}