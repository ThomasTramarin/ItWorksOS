#include <base/sections.h>
#include <hal/port.h>
#include <kernel/error.h>
#include <log/backend.h>

#ifdef CONFIG_QEMU_DEBUGCON

#define DEBUGCON_PORT 0xE9

static int32_t backend_debugcon_write(const struct log_record *record) {
  if (!record || !record->message)
    return -KERR_INVAL;

  for (size_t i = 0; i < record->hdr.length; i++)
    hal_port_write8(DEBUGCON_PORT, record->message[i]);

  return KERR_OK;
}

static struct log_backend backend_debugcon = {
    .name = "debugcon",
    .write = backend_debugcon_write,
};

int32_t __init backend_debugcon_init(void) {
  return log_backend_register(&backend_debugcon);
}

#endif