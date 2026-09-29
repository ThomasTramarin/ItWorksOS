#include <base/sections.h>
#include <kernel/error.h>
#include <klib/bitmap.h>
#include <process/pid.h>

static uint32_t pid_bm_data[BITMAP_ELEMS(PID_MAX)];
static struct bitmap pid_bm = BITMAP_INIT(pid_bm_data, PID_MAX);

pid_t pid_alloc(void) {
  size_t pos;

  if (bitmap_find_zero(&pid_bm, &pos) == false) {
    return -KERR_NOSPC;
  }

  bitmap_set(&pid_bm, pos);

  return (pid_t)pos;
}

int32_t pid_free(pid_t pid) {

  if (pid < PID_FIRST || pid >= PID_MAX)
    return -KERR_INVAL;

  if (!bitmap_test(&pid_bm, pid))
    return -KERR_NOENT;

  bitmap_clear(&pid_bm, pid);

  return KERR_OK;
}

int32_t __init pid_init(void) {
  bitmap_set(&pid_bm, 0);
  return KERR_OK;
}