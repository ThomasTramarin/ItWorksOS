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
 * @brief Statically initializes a ring buffer
 *
 * capacity must be a power of 2
 */
#define RINGBUF_INIT(storage, capacity)                                        \
  (ringbuf_t) { .buf = storage, .cap = capacity, .head = 0, .tail = 0, }

/**
 * @brief Initializes a ring buffer instance
 *
 * @param storage pointer to the pre-allocated memory array
 * @param cap buffer capacity (must be a power of two)
 * @return true on success, false if incorrect parameters are passed
 */
bool ringbuf_init(ringbuf_t *rb, uint8_t *storage, size_t cap);

// --- Utilities ---
bool ringbuf_is_empty(const ringbuf_t *rb);
size_t ringbuf_len(const ringbuf_t *rb);
bool ringbuf_is_full(const ringbuf_t *rb);
size_t ringbuf_available(const ringbuf_t *rb);

/**
 * @brief Writes a byte to the buffer, overwriting the oldest byte if full
 * The function never fails
 *
 * @param data Byte to write
 */
void ringbuf_overwrite(ringbuf_t *rb, uint8_t data);

/**
 * @brief Writes a byte to the buffer if space is available
 *
 * This function never discards existing data
 *
 * @param data Byte to write
 * @return true if the byte was written, false if the buffer was full
 */
bool ringbuf_push(ringbuf_t *rb, uint8_t data);

/**
 * @brief Reads and removes the oldest byte from the buffer
 *
 * @param data Destination for the byte. May be NULL to discard it
 *
 * @return true if the buffer was removed, false if the buffer is empty or
 * invalid
 */
bool ringbuf_pop(ringbuf_t *rb, uint8_t *data);

/**
 * @brief Writes a sequence of bytes to the buffer
 *
 * No bytes are written if there is insufficient free space
 *
 * @param src Source byte array
 * @param len Number of bytes to write
 *
 * @return true if all bytes were written, false if there is insufficient space
 */
bool ringbuf_write(ringbuf_t *rb, const uint8_t *src, size_t len);

/**
 * @brief Reads and removes a sequence of bytes from the buffer
 *
 * Copies up to len bytes into dst and advances tail
 *
 * @param dst Destination buffer
 * @param len Maximum number of bytes to read

 * @return Number of bytes actually read
 */
size_t ringbuf_read(ringbuf_t *rb, uint8_t *dst, size_t len);

/**
 * @brief Copies a sequence of bytes without removing them
 *
 * Reads up to len bytes starting from tail. The buffer state is unchanged:
 * tail is not advanced.
 *
 * @param dst Destination buffer
 * @param len Maximum number of bytes to copy
 *
 * @return Number of bytes actually copied
 */
size_t ringbuf_peek_read(const ringbuf_t *rb, uint8_t *dst, size_t len);

/**
 * @brief Reads a single byte without removing it
 *
 * The index is relative to tail:
 * index 0 identifies the oldest available byte.
 *
 * @param index Logical offset from tail
 * @param data  Destination for the byte
 *
 * @return true if the byte was read, false if the index is out of range
 *         or the parameters are invalid.
 */
bool ringbuf_peek(const ringbuf_t *rb, size_t index, uint8_t *data);

/**
 * @brief Removes bytes without copying them
 *
 * Advances tail by up to len bytes. No data is copied to another buffer.
 *
 * @param len Maximum number of bytes to discard
 *
 * @return Number of bytes actually discarded
 */
size_t ringbuf_discard(ringbuf_t *rb, size_t len);

#endif