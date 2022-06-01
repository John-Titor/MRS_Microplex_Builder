/*
 * Utility library.
 */

#ifndef CORE_LIB_H_
#define CORE_LIB_H_

#include <stdint.h>

#include <intrinsics_hc08.h>

/**
 * Assert that a condition is true.
 */
#define REQUIRE(_cond)                                          \
        do {                                                    \
            if (!(_cond))  __require_abort(__FILE__, __LINE__); \
        } while(0)

#define ABORT()     REQUIRE(0)

extern void __require_abort(const char *file, int line);

/**
 * Save/disable and restore interrupt state
 */
#define ENTER_CRITICAL_SECTION  do { char __interrupt_state = __isflag_int_enabled(); __asm SEI
#define EXIT_CRITICAL_SECTION   if (__interrupt_state) __asm CLI; } while(0)

/**
 * Wrapper for printf().
 *
 * Automatically adds a newline, and sends output via CAN.
 */
extern void print(const char *format, ...);

/**
 * Wrapper for printf().
 *
 * Sends output via CAN.
 */
extern void printn(const char *format, ...);

/**
 * Print a hexdump of a range of memory.
 */
extern void hexdump(uint8_t *addr, unsigned int count);

#endif /* LIB_H_ */
