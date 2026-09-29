#ifndef PROCESS_PID_H
#define PROCESS_PID_H

#include <base/stdint.h>

#define PID_MAX 64

#define PID_FIRST 1

/*
 * The pid number cannot be negative
 * PID 0 is reserved for the idle "process"
 * PID 1 is reserved for the system process
 */

typedef int32_t pid_t;

/**
 * @brief Get an available process ID number
 *
 * @return A positive PID or -KERR_NOSPC if not available
 */
pid_t pid_alloc(void);

/**
 * @brief Free a previously allocated PID number
 *
 * @param pid The PID number to free
 * @return KERR_OK on success or a negative kernel error code
 */
int32_t pid_free(pid_t pid);

int32_t pid_init(void);

#endif