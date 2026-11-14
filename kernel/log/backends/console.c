#include <base/sections.h>
#include <console/console.h>
#include <kernel/error.h>
#include <log/backend.h>

static int32_t backend_console_write(const struct log_record *record) {
  if (!record || !record->message)
    return -KERR_INVAL;

  return console_write(record->message, record->hdr.length);
}

static struct log_backend backend_console = {.name = "console",
                                             .write = backend_console_write};

int32_t __init backend_console_init(void) {
  return log_backend_register(&backend_console);
}