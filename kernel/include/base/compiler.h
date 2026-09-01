#ifndef BASE_COMPILER_H
#define BASE_COMPILER_H

/**
 * @brief Compiler-specific attribute macros
 *
 */

#define __packed __attribute__((packed))
#define __used __attribute__((used))
#define __unused __attribute__((unused))
#define __aligned(n) __attribute__((aligned(n)))

#define __always_inline __attribute__((always_inline))
#define __noinline __attribute__((noinline))

#define __section(name) __attribute__((section(name)))

#define __noreturn __attribute__((noreturn))

#endif