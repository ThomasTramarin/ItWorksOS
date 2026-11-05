#include <klib/ringbuf.h>

bool ringbuf_init(ringbuf_t *rb, uint8_t *storage, size_t cap) {
  if (!rb || !storage || cap < 2 || (cap & (cap - 1)) != 0)
    return false;

  rb->buf = storage;
  rb->cap = cap;
  rb->head = 0;
  rb->tail = 0;

  return true;
}

// --- Utilities ---
bool ringbuf_is_empty(const ringbuf_t *rb) {
  return !rb || rb->head == rb->tail;
}
size_t ringbuf_len(const ringbuf_t *rb) { return rb ? rb->head - rb->tail : 0; }
bool ringbuf_is_full(const ringbuf_t *rb) {
  return rb && (rb->head - rb->tail) == rb->cap;
}
size_t ringbuf_available(const ringbuf_t *rb) {
  return rb ? rb->cap - ringbuf_len(rb) : 0;
}

void ringbuf_overwrite(ringbuf_t *rb, uint8_t data) {
  if (!rb || !rb->buf || rb->cap)
    return;

  if (ringbuf_is_full(rb))
    rb->tail++;

  rb->buf[rb->head & (rb->cap - 1)] = data;
  rb->head++;
}

bool ringbuf_push(ringbuf_t *rb, uint8_t data) {
  if (!rb || !rb->buf || ringbuf_is_full(rb))
    return false;

  ringbuf_overwrite(rb, data);
  return true;
}

bool ringbuf_pop(ringbuf_t *rb, uint8_t *data) {
  if (!rb || !rb->buf || ringbuf_is_empty(rb))
    return false;

  // Copy the oldest byte if a destination was provided
  if (data)
    *data = rb->buf[rb->tail & (rb->cap - 1)];

  // Consume the byte
  rb->tail++;
  return true;
}

bool ringbuf_write(ringbuf_t *rb, const uint8_t *src, size_t len) {
  if (!rb || !rb->buf || (!src && len > 0))
    return false;

  // Do not perform a partial write
  if (len > ringbuf_available(rb))
    return false;

  for (size_t i = 0; i < len; i++) {
    rb->buf[rb->head & (rb->cap - 1)] = src[i];
    rb->head++;
  }

  return true;
}

size_t ringbuf_read(ringbuf_t *rb, uint8_t *dst, size_t len) {
  if (!rb || !rb->buf || (!dst && len > 0))
    return 0;

  size_t available = ringbuf_len(rb);

  if (len > available)
    len = available;

  for (size_t i = 0; i < len; i++) {
    if (dst)
      dst[i] = rb->buf[rb->tail & (rb->cap - 1)];
    rb->tail++;
  }

  return len;
}

size_t ringbuf_peek_read(const ringbuf_t *rb, uint8_t *dst, size_t len) {
  if (!rb || !rb->buf || (!dst && len > 0))
    return 0;

  size_t available = ringbuf_len(rb);

  if (len > available)
    len = available;

  // Copy bytes without changing tail
  for (size_t i = 0; i < len; i++)
    dst[i] = rb->buf[(rb->tail + i) & (rb->cap - 1)];

  return len;
}

bool ringbuf_peek(const ringbuf_t *rb, size_t index, uint8_t *data) {
  if (!rb || !rb->buf || !data || index >= ringbuf_len(rb))
    return false;

  size_t calc_idx = (rb->tail + index) & (rb->cap - 1);

  *data = rb->buf[calc_idx];

  return true;
}

size_t ringbuf_discard(ringbuf_t *rb, size_t len) {
  if (!rb || !rb->buf)
    return 0;

  size_t available = ringbuf_len(rb);

  if (len > available)
    len = available;

  // Consume bytes without copying them
  rb->tail += len;

  return len;
}