#ifndef LOG_PANIC_H
#define LOG_PANIC_H

#include <base/stdarg.h>

void panic(const char *fmt, ...);

void vpanic(const char *fmt, va_list args);

#endif