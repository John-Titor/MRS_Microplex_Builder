/*
 * Timers and timebase.
 */

#ifndef _TIMER_H
#define _TIMER_H

#include <stdint.h>
#include <stdbool.h>

#include <mc9s08dz60.h>

// one-shot timer
typedef struct _timer {
    struct _timer   *_next;
    uint16_t        delay_ms;
} timer_t;

// one-shot or periodic timer callback
typedef struct _timer_call {
    struct _timer_call *_next;
    uint16_t        delay_ms;
    void            (*callback)();  // function to call - must be interrupt-safe
    uint16_t        period_ms;      // tick interval between calls, 0 for one-shot
} timer_call_t;

// high-resolution time
typedef uint32_t microseconds;

// initialize the timers
extern void         time_init(void);

// get the current time
extern microseconds time_us(void);  // costs ~6us

// check whether some time has elapsed
extern bool         time_elapsed_us(microseconds since_us, uint16_t interval_us);

// wait for a period
extern void         time_wait_us(uint16_t delay_us);

// register a one-shot timer
extern void         timer_register(timer_t *timer);

// register a timer callback
extern void         timer_call_register(timer_call_t *call);

// reset a one-shot timer or timer callback
#define timer_reset(_timer, _delay)     \
    do {                                \
        ENTER_CRITICAL_SECTION;         \
        (_timer).delay_ms = _delay;     \
        EXIT_CRITICAL_SECTION;          \
    } while(0)

// test whether a timer or callback has expired
#define timer_expired(_timer)       ((_timer).delay_ms == 0)

// test whether a timer has been registered
#define timer_registered(_timer)    ((_timer)._next != NULL)

#endif // _TIMER_H
