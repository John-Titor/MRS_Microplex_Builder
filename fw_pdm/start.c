/*
 * Manage the start key and starter relay.
 */
#include "defs.h"

PT_DEFINE(start)
{
    static HAL_timer_t  restart_delay;

    pt_begin(pt);

    /* initialise the restart delay timer */
    HAL_timer_register(restart_delay);

    /* turn off the starter relay */
    g_state.starting = 0;
    HAL_pin_set(OUT_START, false);

    /* check whether engine is running */
    if (g_state.engine_rpm > 150) {

        /* yes, prevent starting at least until delay expires */
        HAL_timer_reset(restart_delay, CONFIG_RESTART_DELAY);

        /* turn off the start key LED */
        bk_set_key_led(KEY_START, BK_KEY_COLOR_OFF, 0);

        /* self-reset and try again */
        g_state.start_waiting = 1;
        goto restart;
    }
    g_state.start_waiting = 0;

    /* check for a start-inhibited condition */
    if (!g_state.brake_applied ||               /* brake not applied */
        (g_state.selected_gear != 'P') ||       /* not in Park */
        !HAL_timer_expired(restart_delay) ||    /* engine only just stopped */
        (g_state.dde_switch_mv > CONFIG_DDE_SWITCH_MV)) {   /* DDE relay not energised */

        /* illuminate the start key red to indicate start inhibited */
        bk_set_key_led(KEY_START, BK_KEY_COLOR_RED, 0);

        /* self-reset and try again */
        g_state.start_inhibited = 1;
        goto restart;
    }
    g_state.start_inhibited = 0;

    /* check start button state */
    switch (bk_get_key_event(KEY_START)) {
    case BK_EVENT_LONG_PRESS_1:
    case BK_EVENT_LONG_PRESS_2:
    case BK_EVENT_LONG_PRESS_3:
        /* start button has been pressed; set LED flashing */
        bk_set_key_led(KEY_START, BK_KEY_COLOR_GREEN, 0xaa);

        /* start button has been pressed; turn on the starter relay */
        HAL_pin_set(OUT_START, true);
        g_state.starting = 1;
        break;

    default:
        /* illuminate the start key green to indicate start not inhibited */
        bk_set_key_led(KEY_START, BK_KEY_COLOR_GREEN, 0);

        /* self-reset and try again */
        goto restart;
    }

    /* loop waiting for start button to be released or engine speed over 500rpm */
    for (;;) {

        /* check for start key released or keypad disconnect */
        switch (bk_get_key_event(KEY_START)) {
        case BK_EVENT_DISCONNECTED:
        case BK_EVENT_RELEASE:
        case BK_EVENT_SHORT_PRESS:
            /* self-reset back to not-starting state */
            goto restart;

        default:
            break;
        }

        /* check for engine running */
        if (g_state.engine_rpm > 500) {

            /* prevent starting again before delay expires */
            HAL_timer_reset(restart_delay, CONFIG_RESTART_DELAY);

            /* self-reset back to not-starting state */
            goto restart;
        }

        /* still starting, yield to other threads */
        pt_yield(pt);
    }

    pt_end(pt);

restart:
    pt_reset(pt);
    return;
}
