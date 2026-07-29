#include <klib/ringbuf.h>

void ringbuf_init(ringbuf_t *rb, uint8_t *storage, size_t cap) {
  rb->buf = storage;
  rb->cap = cap;
  rb->head = 0;
  rb->tail = 0;
}

// --- Utilities ---
bool ringbuf_is_empty(ringbuf_t *rb) { return rb->head == rb->tail; }
size_t ringbuf_len(ringbuf_t *rb) { return rb->head - rb->tail; }
bool ringbuf_is_full(ringbuf_t *rb) { return (rb->head - rb->tail) == rb->cap; }
size_t ringbuf_available(ringbuf_t *rb) {
  return rb->cap - (rb->head - rb->tail);
}

void ringbuf_overwrite(ringbuf_t *rb, uint8_t data) {
  if (ringbuf_is_full(rb))
    rb->tail++;

  rb->buf[rb->head & (rb->cap - 1)] = data;
  rb->head++;
}

bool ringbuf_push(ringbuf_t *rb, uint8_t data) {
  if (ringbuf_is_full(rb))
    return false;

  ringbuf_overwrite(rb, data);
  return true;
}

bool ringbuf_pop(ringbuf_t *rb, uint8_t *data) {
  if (ringbuf_is_empty(rb))
    return false;

  if (data)
    *data = rb->buf[rb->tail & (rb->cap - 1)];

  rb->tail++;
  return true;
}