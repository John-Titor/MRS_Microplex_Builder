/*
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
} timer_t;

/**
 *  One-shot or periodic timer callback.
 */
typedef struct _timer_call {
    struct _timer_call *_next;
    uint16_t        delay_ms;
    uint16_t        period_ms;          // tick interval between calls, 0 for one-shot
    void            (*callback)(void);  // function to call - must be interrupt-safe
} timer_call_t;

/**
 * high-resolution time
 */
typedef uint32_t microseconds;

/**
 * initialize the timers
 */
extern void         time_init(void);

/**
 * get the current time
 */
extern microseconds time_us(void);

/**
 * check whether some time has elapsed
 */
extern bool         time_elapsed_us(microseconds since_us, uint16_t interval_us);

/**
 * wait for a period
 */
extern void         time_wait_us(uint16_t delay_us);

/**
 * Register a one-shot timer.
 * 
 * @note does nothing to an already-registered timer.
 */
#define timer_register(_t)      _timer_register(&_t)
extern void         _timer_register(timer_t *timer);

/**
 * Register a timer callback.
 * 
 * @note does nothing to an already-registered callback.
 */
#define timer_call_register(_t) _timer_call_register(&_t)
extern void         _timer_call_register(timer_call_t *call);

/** 
 * Reset a one-shot timer or timer callback
 */
#define timer_reset(_timer, _delay)     \
    do {                                \
        ENTER_CRITICAL_SECTION;         \
        (_timer).delay_ms = _delay;     \
        EXIT_CRITICAL_SECTION;          \
    } while(0)

/**
 * Test whether a timer or callback has expired
 */
#define timer_expired(_timer)           ((_timer).delay_ms == 0)

/**
 * Blocking delay for protothreads
 * 
 * The current thread will be blocked until the delay has expired.
 * 
 * @param pt            The current protothread
 * @param timer         The timer to use
 * @param ms            The number of milliseconds to block
 */
#define pt_delay(pt, timer, ms)                 \
        do {                                    \
            timer_reset(timer, ms);             \
            pt_wait(pt, timer_expired(timer));  \
        } while(0)
