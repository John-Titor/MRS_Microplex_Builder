/** @file
 *
 * Timers and timebase.
 *
 * Using TPM2C2
 *
 * FFCLK is 1MHz, so we run with a /1 prescaler to count microseconds.
 *
 * We maintain a 32-bit timebase which will wrap after ~71 minutes,
 * so code must be careful about absolute time values.
 *
 * A note on time_wait_us: the maximum delay is limited to uint16_t both
 * for efficiency (waiting longer than 64ms is not friendly to other parts
 * of the system) and also to make it safe to use in a critical region;
 * time_us can only handle one wrap before it needs the overflow handler to
 * run and adjust the timebase high word.
 */

#pragma ONCE

#include <stdint.h>
#include <stdbool.h>
#include <mc9s08dz60.h>
#include <lib.h>

/**
 * One-shot timer.
 */
typedef struct _HAL_timer {
    uint16_t            delay_ms;   /**< delay before expiration */
    struct _HAL_timer   *_next;
} HAL_timer_t;

/**
 *  One-shot or periodic timer callback.
 */
typedef struct _HAL_timer_call {
    void (*callback)(void);         /**< function to call - must be interrupt-safe */
    uint16_t        delay_ms;       /**< delay to first call */
    uint16_t        period_ms;      /**< tick interval between calls, 0 for one-shot */
    struct _HAL_timer_call *_next;
} HAL_timer_call_t;

/**
 * high-resolution time
 */
typedef uint32_t HAL_microseconds;

extern void         _HAL_timer_init(void);

/**
 * get the current time
 *
 * @return     Time since system start in microseconds.
 */
extern HAL_microseconds HAL_timer_us(void);

/**
 * check whether some time has elapsed
 *
 * @param[in]  since_us     Epoch to calculate from.
 * @param[in]  interval_us  Interval to check.
 *
 * @return     True if the interval has elapsed, false otherwise.
 */
extern bool         HAL_timer_elapsed_us(HAL_microseconds since_us, uint16_t interval_us);

/**
 * wait for a period
 *
 * @param[in]  delay_us  Delay to wait for.
 */
extern void         HAL_timer_wait_us(uint16_t delay_us);

/**
 * Register a one-shot timer.
 *
 * @note       does nothing to an already-registered timer.
 *
 * @param      _t    Timer to register.
 */
#define HAL_timer_register(_t) _HAL_timer_register(&_t)
extern void         _HAL_timer_register(HAL_timer_t *timer);

/**
 * Register a timer callback.
 *
 * @note       does nothing to an already-registered callback.
 *
 * @param      _t    Callback timer to register.
 */
#define HAL_timer_call_register(_t) _HAL_timer_call_register(&_t)
extern void         _HAL_timer_call_register(HAL_timer_call_t *call);

/**
 * Reset a one-shot timer or timer callback.
 *
 * @param      _timer  Timer to reset.
 * @param      _delay  New timeout value to set.
 *
 * @return     { description_of_the_return_value }
 */
#define HAL_timer_reset(_timer, _delay) \
    do {                                \
        ENTER_CRITICAL_SECTION;         \
        (_timer).delay_ms = _delay;     \
        EXIT_CRITICAL_SECTION;          \
    } while(0)

/**
 * Test whether a timer or callback has expired
 *
 * @param      _timer  Timer to check.
 *
 * @return     true if the timer or callback has expired, false otherwise.
 */
#define HAL_timer_expired(_timer)           ((_timer).delay_ms == 0)
