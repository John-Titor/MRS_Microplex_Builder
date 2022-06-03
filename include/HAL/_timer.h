/** @file
 *
 * Timers and timebase.
 */

#pragma ONCE

#include <stdint.h>
#include <stdbool.h>

#include <mc9s08dz60.h>

/**
 * One-shot timer.
 */
typedef struct _timer {
    struct _timer   *_next;
    uint16_t        delay_ms;
} HAL_timer_t;

/**
 *  One-shot or periodic timer callback.
 */
typedef struct _timer_call {
    struct _timer_call *_next;
    uint16_t        delay_ms;
    uint16_t        period_ms;          // tick interval between calls, 0 for one-shot
    void            (*callback)(void);  // function to call - must be interrupt-safe
} HAL_timer_call_t;

/**
 * high-resolution time
 */
typedef uint32_t HAL_microseconds;

extern void         _HAL_timer_init(void);

/**
 * get the current time
 */
extern HAL_microseconds HAL_timer_us(void);

/**
 * check whether some time has elapsed
 */
extern bool         HAL_timer_elapsed_us(HAL_microseconds since_us, uint16_t interval_us);

/**
 * wait for a period
 */
extern void         HAL_timer_wait_us(uint16_t delay_us);

/**
 * Register a one-shot timer.
 *
 * @note does nothing to an already-registered timer.
 */
#define HAL_timer_register(_t) _HAL_timer_register(&_t)
extern void         _HAL_timer_register(HAL_timer_t *timer);

/**
 * Register a timer callback.
 *
 * @note does nothing to an already-registered callback.
 */
#define HAL_timer_call_register(_t) _HAL_timer_call_register(&_t)
extern void         _HAL_timer_call_register(HAL_timer_call_t *call);

/**
 * Reset a one-shot timer or timer callback
 */
#define HAL_timer_reset(_timer, _delay) \
    do {                                \
        ENTER_CRITICAL_SECTION;         \
        (_timer).delay_ms = _delay;     \
        EXIT_CRITICAL_SECTION;          \
    } while(0)

/**
 * Test whether a timer or callback has expired
 */
#define HAL_timer_expired(_timer)           ((_timer).delay_ms == 0)
