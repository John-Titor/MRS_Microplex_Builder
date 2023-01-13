/*
 * Keypad logic.
 *
 * All keypad handling (except for the start button) is handled here.
 */
#include "defs.h"

PT_DEFINE(keypad)
{
    pt_begin(pt);

    bk_set_key_led(KEY_LIGHTS, BK_KEY_COLOR_WHITE, 0);
    bk_set_key_led(KEY_RAIN, BK_KEY_COLOR_WHITE, 0);

    for (;;) {
        /* get keypad event */
        uint8_t code = bk_get_event();

        /* if it's a key event */
        if (code < BK_EVENT_DISCONNECTED) {
            uint8_t key = code & BK_KEY_MASK;
            uint8_t event = code & BK_EVENT_MASK;

            /* lighting toggles */
            if ((key == KEY_LIGHTS) && (event == BK_EVENT_SHORT_PRESS)) {
                g_state.lights_on = (g_state.lights_on ? 0 : 1);
                bk_set_key_led(KEY_LIGHTS, g_state.lights_on ? BK_KEY_COLOR_BLUE : BK_KEY_COLOR_WHITE, 0);

            }

            if ((key == KEY_RAIN) && (event == BK_EVENT_SHORT_PRESS)) {
                g_state.rain_on = (g_state.rain_on ? 0 : 1);
                bk_set_key_led(KEY_RAIN, g_state.lights_on ? BK_KEY_COLOR_BLUE : BK_KEY_COLOR_WHITE, 0);
            }
        }

        pt_yield(pt);
    }

    pt_end(pt);
}
