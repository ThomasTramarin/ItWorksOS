#include <base/sections.h>
#include <klib/memory.h>
#include <klib/ringbuf.h>
#include <log/syslog.h>

// Logs Ring Buffer
static uint8_t syslog_storage[SYSLOG_BUFFER_SIZE];
static ringbuf_t syslog_rb;
static bool syslog_ready = false;

void __init syslog_init(void) {
  ringbuf_init(&syslog_rb, syslog_storage, SYSLOG_BUFFER_SIZE);
  syslog_ready = true;
}

void syslog_putc(char c) {
  if (!syslog_ready)
    return;
  ringbuf_overwrite(&syslog_rb, (uint8_t)c);
}

void syslog_write(const char *str) {
  while (*str) {
    syslog_putc(*str);
    str++;
  }
}

size_t syslog_len(void) {
  if (!syslog_ready)
    return 0;

  return ringbuf_len(&syslog_rb);
}

size_t syslog_read(char *dst, size_t size) {
  // tail represents the reader starting position
  // tail is not incremented so this function does not consume bytes
  // tail is only incremented by ringbuf_overwrite when the buffer becomes full

  if (!syslog_ready || !dst || size == 0)
    return 0;

  size_t len = ringbuf_len(&syslog_rb);

  // leave space for '\0'
  if (len >= size) {
    len = size - 1;
  }

  // copy len bytes, starting from tail, without consume them
  for (size_t i = 0; i < len; i++) {
    uint8_t c;

    if (ringbuf_peek(&syslog_rb, i, &c)) {
      dst[i] = c;
    }
  }

  dst[len] = '\0';

  return len;
}