#ifndef LOG_SYSLOG_H
#define LOG_SYSLOG_H

#include <base/stdbool.h>
#include <base/stddef.h>

#define SYSLOG_BUFFER_SIZE 4096

/**
 * @file syslog.h
 * @brief Kernel in-memory log buffer interface.
 */

/**
 * @brief Initializes the kernel syslog ring buffer.
 * Must be called during early boot before any logs are generated.
 */
void syslog_init(void);

/**
 * @brief Appends a single character to the kernel log buffer.
 * Overwrites the oldest log entry if the buffer is full.
 */
void syslog_putc(char c);

/**
 * @brief Appends a null-terminated string to the kernel log buffer.
 */
void syslog_write(const char *str);

/**
 * @brief Gets the current number of bytes stored in the log buffer.
 *
 * @return Number of stored bytes.
 */
size_t syslog_len(void);

/**
 * @brief Reads logs from the buffer into a destination array without consuming
 * them.
 *
 * @param dst pointer to the output buffer where logs will be copied.
 * @param size maximum number of bytes to copy.
 * @return number of bytes actually copied into dst.
 */
size_t syslog_read(char *dst, size_t size);

#endif
