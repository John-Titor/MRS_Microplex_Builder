/*
 * Timers and timebase
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
 *
 */

#include <stdlib.h>
#include <stdint.h>

#include <lib.h>
#include <HAL/_timer.h>

#define _timer_registered(_timer)    ((_timer)._next != NULL)

#define TIMER_LIST_END      (HAL_timer_t *)4
#define TIMER_CALL_LIST_END (HAL_timer_call_t *)8

static HAL_timer_t      *timer_list = TIMER_LIST_END;
static HAL_timer_call_t *timer_call_list = TIMER_CALL_LIST_END;
static volatile uint16_t timebase_high_word;

void
_HAL_timer_init(void)
{
    TPM2SC = 0;
    TPM2SC_CLKSx = 2;   // select fixed clock
    TPM2SC_PS = 0;      // /1 prescaler
    TPM2MOD = 0;        // clear modulus
    TPM2CNT = 0;        // clear counter
    TPM2SC_TOIE = 1;    // enable overflow interrupt

    TPM2C1SC = 0;
    TPM2C1SC_MS1A = 1;  // output compare
    TPM2C0V = 1000;     // set initial deadline
    TPM2C1SC_CH1IE = 1; // enable interrupt
}

HAL_microseconds
HAL_timer_us(void)
{
    uint32_t        tv;
    bool            overflow;

    // loop if we detect overflow, as it's possible that the high
    // and low words we fetched are out of sync
    do {
        ENTER_CRITICAL_SECTION;

        // get the "current" time value
        tv = ((uint32_t)timebase_high_word << 16) + TPM2CNT;

        // check whether we might have raced with overflow...
        overflow = TPM2SC & TPM2SC_TOF_MASK;

        EXIT_CRITICAL_SECTION;

    } while (overflow);

    return tv;
}


bool
HAL_timer_elapsed_us(HAL_microseconds since_us, uint16_t interval_us)
{
    return (HAL_timer_us() - since_us) >= interval_us;
}

void
HAL_timer_wait_us(uint16_t delay_us)
{
    HAL_microseconds then = HAL_timer_us();

    while (!HAL_timer_elapsed_us(then, delay_us)) {
    }
}

static void
__interrupt VectorNumber_Vtpm2ovf
Vtpm2ovf_handler(void)
{
    timebase_high_word++;
    TPM2SC_TOF = 0;
}

void
_HAL_timer_register(HAL_timer_t *timer)
{
    ENTER_CRITICAL_SECTION;

    REQUIRE(timer != NULL);

    if (!_timer_registered(*timer)) {
        // singly-linked insertion at head
        timer->_next = timer_list;
        timer_list = timer;
    }

    EXIT_CRITICAL_SECTION;
}

void
_HAL_timer_call_register(HAL_timer_call_t *call)
{
    ENTER_CRITICAL_SECTION;

    REQUIRE(call != NULL);
    REQUIRE(call->callback != NULL);

    if (!_timer_registered(*call)) {

        // singly-linked insertion at head
        call->_next = timer_call_list;
        timer_call_list = call;
    }

    EXIT_CRITICAL_SECTION;
}

static void
__interrupt VectorNumber_Vtpm2ch1
Vtpm2ch1_handler(void)
{
    HAL_timer_t *t;
    HAL_timer_call_t *tc;

    // re-set compare for next tick
    // must update TPM2C1V *after* clearing the interrupt
#pragma MESSAGE DISABLE C2705
    TPM2C1SC &= ~TPM2C1SC_CH1F_MASK;
#pragma MESSAGE DEFAULT C2705
    TPM2C1V += 1000;

    // update timers
    for (t = timer_list;
            t != TIMER_LIST_END;
            t = t->_next) {
        if (t->delay_ms > 0) {
            t->delay_ms--;
        }
    }

    // run timer calls
    for (tc = timer_call_list;
            tc != TIMER_CALL_LIST_END;
            tc = tc->_next) {

        // if the call is active...
        if (tc->delay_ms > 0) {
            // and the delay expires...
            if (--tc->delay_ms == 0) {
                // run the callback
                tc->callback();
                // and reload the delay (or leave it at
                // zero for a one-shot)
                tc->delay_ms = tc->period_ms;
            }
        }
    }

    // verify that we have not run into the next tick
    REQUIRE(!TPM2C1SC_CH1F);
}
