/*
 * CAN lighting message logic.
 */
#include "defs.h"

PT_DEFINE(lights)
{
    static HAL_timer_t rain_blink;
    pt_begin(pt);
    HAL_timer_register(rain_blink);

    for (;;) {

        /* tail lights on / off according to CAN message */
        if (g_state.lights_on != g_state.lights_requested) {
            HAL_pin_set_duty(OUT_TAIL_LIGHTS, g_state.lights_requested ? CONFIG_TAIL_INTENSITY : 0);
            g_state.lights_on = g_state.lights_requested;
        }

#ifdef OUT_REVERSE
        /* reverse signal on / off according to CAN message */
        if (g_state.reverse_on != g_state.reverse_requested) {
            HAL_pin_set(OUT_REVERSE, g_state.reverse_requested ? true : false);
            g_state.reverse_on = g_state.reverse_requested;
        }
#endif

#ifdef OUT_RAIN_LIGHT
        /* rain light blink / off according to CAN message */
        if (g_state.rain_requested) {
            if (HAL_timer_expired(rain_blink)) {
                if (g_state.rain_on) {
                    HAL_pin_set(OUT_RAIN_LIGHT, false);
                    g_state.rain_on = 0;
                } else {
                    HAL_pin_set(OUT_RAIN_LIGHT, true);
                    g_state.rain_on = 1;
                }

                HAL_timer_reset(rain_blink, CONFIG_RAIN_FLASH_PERIOD);
            }
        } else {
            HAL_pin_set(OUT_RAIN_LIGHT, false);
        }
#endif

        pt_yield(pt);
    }

    pt_end(pt);
}
