#ifndef KLIB_RINGBUF_H
#define KLIB_RINGBUF_H

#include <base/stdbool.h>
#include <base/stddef.h>

/**
 * @file ringbuf.h
 * @brief Generic Byte Ring Buffer implementation
 *
 * Characteristics:
 *  - the capacity must be a power of two
 *  - it supports both non-overwriting and overwriting operations
 *  - head and tail grow continuously without resetting. The index
 *    is calculated using the AND operator
 */

typedef struct ringbuf_s {
  uint8_t *buf; // pointer to the array in memory
  size_t cap;   // max capacity (must be a power of 2)
  size_t head;  // head pointer
  size_t tail;  // tail pointer
} ringbuf_t;

/**
 * @brief Initializes a ring buffer instance
 * The function never fails
 *
 * @param storage pointer to the pre-allocated memory array
 * @param cap buffer capacity (must be a power of two)
 */
void ringbuf_init(ringbuf_t *rb, uint8_t *storage, size_t cap);

// --- Utilities ---
bool ringbuf_is_empty(ringbuf_t *rb);
size_t ringbuf_len(ringbuf_t *rb);
bool ringbuf_is_full(ringbuf_t *rb);
size_t ringbuf_available(ringbuf_t *rb);

/**
 * @brief Writes a byte to the buffer, overwriting the oldest byte if full
 * The function never fails
 */
void ringbuf_overwrite(ringbuf_t *rb, uint8_t data);

/**
 * @brief Writes a byte to the buffer if space is available
 * @return true if the byte was written, false if the buffer was full
 */
bool ringbuf_push(ringbuf_t *rb, uint8_t data);

/**
 * @brief Reads and removes the oldest byte from the buffer
 * @return true if the buffer was non-empty, false if the buffer was empty (data
 * is not touched in this last case)
 */
bool ringbuf_pop(ringbuf_t *rb, uint8_t *data);

/**
 * @brief Reads an element (byte) relative to the tail, without move it
 *
 */
bool ringbuf_peek(ringbuf_t *rb, size_t index, uint8_t *data);

#endif