#ifndef LOG_SYSLOG_H
#define LOG_SYSLOG_H

#include <base/stdbool.h>
#include <base/stddef.h>
#include <log/log.h>

/**
 * @file syslog.h
 * @brief Kernel in-memory log buffer interface.
 */

/**
 * @brief Initializes the kernel log system
 *
 * @return KERR_OK on success, negative kernel error code on failure
 */
int32_t syslog_init(void);

/**
 * @brief Appends a null-terminated string to the kernel log buffer
 * @return KERR_OK on success, negative kernel error code on failure
 */
int32_t syslog_write(enum log_level level, const char *str);

/**
 * @brief Gets the current number of bytes stored in the log buffer
 *
 * Includes record headers and message payloads
 *
 * @return Number of stored bytes
 */
size_t syslog_len(void);

#endif
