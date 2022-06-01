// Based on https://github.com/zserge/pt
//
#ifndef _PT_H
#define _PT_H

#include <stddef.h>

/* Protothread status values */
#define PT_STATUS_BLOCKED   0
#define PT_STATUS_FINISHED  -1
#define PT_STATUS_YIELDED   -2

/*
 * Local continuation based on switch/case and line numbers.
 */
struct pt {
    unsigned int  label;
    signed char   status;
};
#define pt_init()                                                               \
    { .label = 0, .status = 0 }
#define pt_begin(pt)                                                            \
    switch ((pt)->label) {                                                      \
    case 0:
#define pt_label(pt, stat)                                                      \
    do {                                                                        \
        (pt)->label = __LINE__;                                                 \
        (pt)->status = (stat);                                                  \
    case __LINE__:;                                                             \
    } while (0)
#define pt_end(pt)                                                              \
    pt_label(pt, PT_STATUS_FINISHED);                                           \
    }
#define pt_reset(pt)                                                            \
    do {                                                                        \
        (pt)->label = 0;                                                        \
        (pt)->status = 0;                                                       \
    } while(0)

/*
 * Core protothreads API
 */
#define pt_status(pt) (pt)->status

#define pt_running(pt) (pt_status(pt) != PT_STATUS_FINISHED)

#define pt_wait(pt, cond)                                                       \
    do {                                                                        \
        pt_label(pt, PT_STATUS_BLOCKED);                                        \
        if (!(cond)) {                                                          \
            return;                                                             \
        }                                                                       \
    } while (0)

#define pt_delay(pt, timer, ms)                                                 \
    do {                                                                        \
        timer_reset(timer, ms);                                                 \
        pt_wait(pt, timer_expired(timer));                                      \
    } while(0)

#define pt_yield(pt)                                                            \
    do {                                                                        \
        pt_label(pt, PT_STATUS_YIELDED);                                        \
        if (pt_status(pt) == PT_STATUS_YIELDED) {                               \
            (pt)->status = PT_STATUS_BLOCKED;                                   \
            return;                                                             \
        }                                                                       \
    } while (0)

#define pt_exit(pt, stat)                                                       \
    do {                                                                        \
        pt_label(pt, stat);                                                     \
        return;                                                                 \
    } while (0)

#endif // _PT_H
