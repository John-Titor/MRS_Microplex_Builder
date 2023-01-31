/*
 * Keypad logic.
 *
 * All keypad handling (except for the start button) is here.
 */
#include "defs.h"

static bool
handle_light_press(uint8_t event)
{
    if (event == BK_EVENT_SHORT_PRESS) {
        if (g_state.lights_on) {
            g_state.lights_on = 0;
        } else {
            g_state.lights_on = 1;
        }
    }
    return g_state.lights_on ? true : false;
}

static bool
handle_rain_press(uint8_t event)
{
    if (event == BK_EVENT_SHORT_PRESS) {
        if (g_state.rain_on) {
            g_state.rain_on = 0;
        } else {
            g_state.rain_on = 1;
        }
    }
    return g_state.rain_on ? true : false;
}

static const struct {
    /* key */
    uint8_t key;
    /* color to set when 'off' */
    uint8_t color_off;
    /* color to set when 'on' (also while pressed) */
    uint8_t color_on;
    /* handler for event - returns true if 'on', false if 'off' */
    bool    (*action)(uint8_t event);
} key_actions[] = {
    { KEY_LIGHTS,   BK_KEY_COLOR_WHITE, BK_KEY_COLOR_GREEN, handle_light_press },
    { KEY_RAIN,     BK_KEY_COLOR_WHITE, BK_KEY_COLOR_YELLOW, handle_rain_press },
    { BK_EVENT_NONE, 0, 0, NULL }
};

PT_DEFINE(keypad)
{
    pt_begin(pt);

    bk_set_key_led(KEY_LIGHTS, BK_KEY_COLOR_WHITE, 0);
    bk_set_key_led(KEY_RAIN, BK_KEY_COLOR_WHITE, 0);
    bk_set_key_intensity(BK_MAX_INTENSITY);

    for (;;) {
        /* get keypad event */
        uint8_t code = bk_get_event();

        g_state.keypad_active = (code != BK_EVENT_DISCONNECTED);

        if (code < BK_EVENT_DISCONNECTED) {
            /* got an event from the keypad */
            uint8_t key = code & BK_KEY_MASK;
            uint8_t event = code & BK_EVENT_MASK;
            uint8_t i;

            /* scan handlers */
            for (i = 0; key_actions[i].key != BK_EVENT_NONE; i++) {
                if (key == key_actions[i].key) {
                    bk_set_key_led(key, key_actions[i].action(event) ? key_actions[i].color_on : key_actions[i].color_off, 0);
                    break;
                }
            }
        }

        pt_yield(pt);
    }

    pt_end(pt);
}
