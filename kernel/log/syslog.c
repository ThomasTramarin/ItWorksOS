#include <base/sections.h>
#include <kernel/error.h>
#include <klib/memory.h>
#include <klib/ringbuf.h>
#include <klib/string.h>
#include <log/backend.h>
#include <log/log.h>
#include <log/syslog.h>

#define CONFIG_LOG_LEVEL LOG_DEBUG

/*
 * The size must be at least
 */
#define SYSLOG_BUFFER_SIZE 4096

// Logs Ring Buffer
static uint8_t syslog_storage[SYSLOG_BUFFER_SIZE];
static ringbuf_t syslog_rb = RINGBUF_INIT(syslog_storage, SYSLOG_BUFFER_SIZE);
static uint32_t sequence = 0; // current sequence number

int32_t backend_debugcon_init(void);
int32_t backend_console_init(void);

int32_t __init syslog_init(void) {
  int32_t ret;

  // registration of log backends
  ret = backend_console_init();
  if (ret < 0)
    return ret;

#ifdef CONFIG_QEMU_DEBUGCON
  ret = backend_debugcon_init();
  if (ret < 0)
    return ret;
#endif

  return KERR_OK;
}

static bool syslog_make_room(size_t required) {
  if (required > SYSLOG_BUFFER_SIZE)
    return false;

  while (ringbuf_available(&syslog_rb) < required) {
    struct log_record_header oldest;

    // Read the oldest record header.
    if (ringbuf_peek_read(&syslog_rb, (uint8_t *)&oldest, sizeof(oldest)) !=
        sizeof(oldest))
      return false;

    // Validate the record size.
    if (oldest.length > SYSLOG_BUFFER_SIZE - sizeof(oldest))
      return false;

    size_t oldest_size = sizeof(oldest) + oldest.length;

    if (oldest_size > ringbuf_len(&syslog_rb))
      return false;

    // Discard the entire oldest record.
    if (ringbuf_discard(&syslog_rb, oldest_size) != oldest_size)
      return false;
  }

  return true;
}

int32_t syslog_write(enum log_level level, const char *str) {
  if (!str)
    return -KERR_INVAL;

  if (level < CONFIG_LOG_LEVEL)
    return KERR_OK;

  size_t msg_len = strlen(str);

  struct log_record_header header = {
      .seq = sequence,
      .level = level,
      .length = msg_len,
  };

  struct log_record dispatch_record = {
      .hdr =
          {
              .seq = header.seq,
              .level = header.level,
              .length = header.length,
          },
      .message = str,
  };

  // Dispatch immediately, independently of storage.
  (void)log_backend_dispatch(&dispatch_record);

  sequence++;

  // Check whether the record can fit in the history buffer.
  if (msg_len > SYSLOG_BUFFER_SIZE - sizeof(header))
    return -KERR_NOSPC;

  size_t record_len = sizeof(header) + msg_len;

  if (!syslog_make_room(record_len))
    return -KERR_NOSPC;

  // Store the record.
  if (!ringbuf_write(&syslog_rb, (const uint8_t *)&header, sizeof(header)))
    return -KERR_NOSPC;

  if (!ringbuf_write(&syslog_rb, (const uint8_t *)str, msg_len))
    return -KERR_NOSPC;

  return KERR_OK;
}

size_t syslog_len(void) { return ringbuf_len(&syslog_rb); }
