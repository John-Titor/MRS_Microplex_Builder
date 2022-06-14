/** @file
 *
 * Utility library.
 */

#pragma ONCE

#include <stdint.h>
#include <intrinsics_hc08.h>

/**
 * Assert that a condition is true.
 *
 * @param _cond     Condition which must be true.
 */
#define REQUIRE(_cond)                                      \
    do {                                                    \
        if (!(_cond))  __require_abort(__FILE__, __LINE__); \
    } while(0)

/** Force a system abort and restart */
#define ABORT()     REQUIRE(0)

extern void __require_abort(const char *file, int line);

/**
 * Save and disable interrupt enable state.
 */
#define ENTER_CRITICAL_SECTION  \
    do { char __interrupt_state = __isflag_int_enabled(); __asm SEI

/**
 * Restore interrupt enable state.
 */
#define EXIT_CRITICAL_SECTION   \
    if (__interrupt_state) __asm CLI; } while(0)

/**
 * Wrapper for printf().
 *
 * Automatically adds a newline, and sends output via CAN.
 *
 * @param[in]  format     Format string to process.
 * @param[in]  ...        Arguments as required by the format.
 */
extern void print(const char *format, ...);

/**
 * Wrapper for printf().
 *
 * Sends output via CAN.
 *
 * @param[in]  format     Format string to process.
 * @param[in]  ...        Arguments as required by the format.
 */
extern void printn(const char *format, ...);

/**
 * Print a hexdump of a range of memory.
 *
 * @param      addr   Starting address.
 * @param[in]  count  Count in bytes.
 */
extern void hexdump(uint8_t *addr, unsigned int count);
