
/**
 * @file va.h
 *
 * @brief Implementation of C variadic arguments
 *
 * Provides the va_list, va_start, va_arg and va_end macros used to access
 * variable-length argument lists (e.g. functions like printf)
 *
 * Assumes:
 *  - x86 32-bit architecture
 *  - cdecl calling convention
 *  - function arguments are passed on the stack
 *
 * Limitations:
 *  - Does not support x86_64 (register-based arguments)
 *  - Undefined Behavior if incorrect types are used
 *
 * Usage Example:
 *   va_list args;
 *   va_start(args, last_named_param);
 *   int x = va_arg(args, int);
 *   va_end(args);
 */

typedef char *va_list;

#define va_start(ap, last) (ap = (va_list)((char *)&(last) + sizeof(last)))

#define va_arg(ap, type) (*(type *)((ap += sizeof(type)) - sizeof(type)))

#define va_end(ap) ((void)(ap = (va_list)0))