/** @file
 *
 * Protothreads.
 *
 * Based on https://github.com/zserge/pt and cut down to just the bare necessities.
 */

#pragma ONCE

#include <HAL/_timer.h>

/* Protothread status values */
#define PT_STATUS_BLOCKED   0
#define PT_STATUS_FINISHED  1
#define PT_STATUS_YIELDED   2

/* disable "removed dead code" */
#pragma MESSAGE DISABLE C5660

/**
 * Local continuation using Duff's Device.
 */
struct pt {
    unsigned int  label: 13;
    unsigned int  status: 3;
};
#define pt_init()               \
    { .label = 0, .status = 0 }

/**
 * Protothread start.
 *
 * Must be at top level in the protothread function, before any other pt_*
 * functions are called.
 * Any code preceding pt_begin will run every time the thread is run.
 */
#define pt_begin(pt)            \
    switch ((pt)->label) {      \
    case 0:

#define pt_label(pt, stat)      \
    do {                        \
        (pt)->label = __LINE__; \
        (pt)->status = (stat);  \
    case __LINE__:;             \
    } while (0)

/**
 * Protothread end.
 *
 * Must be the last thing at the top level in the protothread function.
 */
#define pt_end(pt)                      \
    pt_label(pt, PT_STATUS_FINISHED);   \
    }

/**
 * Reset a protothread.
 *
 * When the protothread next runs, it will start over.
 *
 * @param pt            The protothread to reset.
 */
#define pt_reset(pt)        \
    do {                    \
        (pt)->label = 0;    \
        (pt)->status = 0;   \
    } while(0)

/*
 * Core protothreads API
 */
#define pt_status(pt) (pt)->status

/**
 * Test whether a protothread is still running.
 *
 * @param pt            The protothread to test.
 */
#define pt_running(pt) (pt_status(pt) != PT_STATUS_FINISHED)

/**
 * Stop a protothread.
 *
 * @param pt            The protothread to stop.
 */
#define pt_stop(pt) do { (pt)->status = PT_STATUS_FINISHED; } while(0)

/**
 * Wait until a condition is satisfied.
 *
 * @param pt            The current protothread.
 * @param cond          The condition to be tested. Will be evaluated once
 *                      each time the protothread is run until it returns true.
 */
#define pt_wait(pt, cond)                   \
    do {                                    \
        pt_label(pt, PT_STATUS_BLOCKED);    \
        if (!(cond)) {                      \
            return;                         \
        }                                   \
    } while (0)

/**
 * Yield the current timeslice.
 *
 * Execution will resume at this point the next time the protothread is run.
 *
 * @param pt            The current protothread.
 */
#define pt_yield(pt)                                \
    do {                                            \
        pt_label(pt, PT_STATUS_YIELDED);            \
        if (pt_status(pt) == PT_STATUS_YIELDED) {   \
            (pt)->status = PT_STATUS_BLOCKED;       \
            return;                                 \
        }                                           \
    } while (0)

#define pt_exit(pt, stat)   \
    do {                    \
        pt_label(pt, stat); \
        return;             \
    } while (0)


/**
 * Blocking delay.
 *
 * The current thread will be blocked until the delay has expired.
 *
 * @param pt            The current protothread
 * @param timer         The timer to use
 * @param ms            The number of milliseconds to block
 */
#define pt_delay(pt, timer, ms)                 \
    do {                                        \
        HAL_timer_reset(timer, ms);             \
        pt_wait(pt, HAL_timer_expired(timer));  \
    } while(0)

/**
 * Declare a protothread by name.
 * 
 * Declares both the protothread function and the thread structure.
 * 
 * @param _name         Protothread name.
 */
#define PT_DECLARE(_name)                       \
    extern struct pt __pt_ ## _name;            \
    extern void pt_ ## _name (struct pt *pt)

/**
 * Heads the definition of a protothread function.
 * 
 * Should be followed by the body of the function. Also defines the
 * thread structure.
 * 
 * @param _name         Protothread name.
 */
#define PT_DEFINE(_name)                        \
    struct pt __pt_ ## _name;                   \
    void pt_ ## _name (struct pt *pt)

/**
 * Run a protothread by name.
 *
 * Calls the thread function and passes the thread structure.
 * 
 * @param _name         Protothread name.
 */
#define PT_RUN(_name)       pt_ ## _name(&__pt_ ## _name)

/**
 * Reset a protothread by name.
 * 
 * Causes the named protothread to restart the next time it is run.
 *
 * @param _name         Protothread name.
 */
#define PT_RESET(_name)     pt_reset(&__pt_ ## _name)
