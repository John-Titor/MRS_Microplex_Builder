/*
 * Brake light logic.
 */
#include "defs.h"

PT_DEFINE(brake)
{
    static HAL_timer_t flash_timer;
    static HAL_timer_t idle_timer;
    static bool flash_state;
    static uint8_t flash_count;
    static uint8_t intensity;

    pt_begin(pt);
    HAL_timer_register(flash_timer);
    HAL_timer_register(idle_timer);

    /* if CAN contact has been lost / regained, we reset back to here */
    while (g_can_idle) {
        /* alternate flash left/right brake lights at full brightness */
        HAL_pin_set(OUT_BRAKE_L, flash_state ? 1 : 0);
        HAL_pin_set(OUT_BRAKE_R, flash_state ? 0 : 1);
        flash_state = !flash_state;
        pt_delay(pt, flash_timer, CONFIG_BRAKE_FAILSAFE_PERIOD);
    }

    /* if engine is not running */
    while (!g_state.engine_running) {
        /* quick left-then-right flash every 4s */
        HAL_pin_set(OUT_BRAKE_L, flash_state ? 1 : 0);
        HAL_pin_set(OUT_BRAKE_R, flash_state ? 0 : 1);
        flash_state = !flash_state;
        pt_delay(pt, flash_timer, CONFIG_BRAKE_REMINDER_PERIOD);

        if (flash_state) {
            HAL_pin_set(OUT_BRAKE_R, 0);
            pt_delay(pt, flash_timer, CONFIG_BRAKE_REMINDER_INTERVAL);
        }
    }

    /* turn brake lights off and wait for brake to be applied */
    HAL_pin_set(OUT_BRAKE_L, 0);
    HAL_pin_set(OUT_BRAKE_R, 0);
    pt_wait(pt, g_state.brake_applied);

    /* select intensity */
    intensity = g_state.lights_requested ? CONFIG_BRAKE_INTENSITY_LOW :
                CONFIG_BRAKE_INTENSITY_HIGH;

    /* do attention getter? */
    if (HAL_timer_expired(idle_timer)) {
        flash_state = false;

        for (flash_count = 0; flash_count < (CONFIG_BRAKE_ATTENTION_CYCLES * 2); flash_count++) {
            HAL_pin_set_duty(OUT_BRAKE_L, flash_state ? intensity : 0);
            HAL_pin_set_duty(OUT_BRAKE_R, flash_state ? 0 : intensity);
            flash_state = flash_state ? false : true;
            pt_delay(pt, flash_timer, CONFIG_BRAKE_ATTENTION_PERIOD);
        }
    }

    /* turn brake lights on and wait for brake to be released */
    HAL_pin_set_duty(OUT_BRAKE_L, intensity);
    HAL_pin_set_duty(OUT_BRAKE_R, intensity);
    pt_wait(pt, !g_state.brake_applied);

    /* reset idle timer */
    HAL_timer_reset(idle_timer, CONFIG_BRAKE_ATTENTION_DELAY);

    /* and reset */
    pt_reset(pt);
    return;

    pt_end(pt);
}