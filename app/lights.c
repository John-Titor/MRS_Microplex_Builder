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
        if (g_light_status.lights_on != g_light_status.lights_requested) {
            HAL_pin_set_duty(OUT_TAIL_LIGHTS, g_light_status.lights_requested ? CONFIG_TAIL_INTENSITY : 0);
            g_light_status.lights_on = g_light_status.lights_requested;
        }

        /* rain light blink / off according to CAN message */
        if (g_light_status.rain_requested) {
            if (HAL_timer_expired(rain_blink)) {
                if (g_light_status.rain_on) {
                    HAL_pin_set(OUT_RAIN_LIGHT, false);
                    g_light_status.rain_on = 0;
                } else {
                    HAL_pin_set(OUT_RAIN_LIGHT, true);
                    g_light_status.rain_on = 1;
                }

                HAL_timer_reset(rain_blink, CONFIG_RAIN_FLASH_PERIOD);
            }
        } else {
            HAL_pin_set(OUT_RAIN_LIGHT, false);
        }

        pt_yield(pt);
    }

    pt_end(pt);
}
