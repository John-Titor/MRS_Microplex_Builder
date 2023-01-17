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
    g_state.brake_on = 0;
    g_state.brake_requested = 0;

    /* if CAN contact has been lost / regained, we reset back to here */
    while (g_state.can_idle) {
        /* alternate flash left/right brake lights at full brightness */
        HAL_pin_set(OUT_BRAKE_L, flash_state ? 1 : 0);
        HAL_pin_set(OUT_BRAKE_R, flash_state ? 0 : 1);
        flash_state = !flash_state;
        pt_delay(pt, flash_timer, CONFIG_BRAKE_FAILSAFE_PERIOD);
    }

    /* turn brake lights off and do wait-for-brake behaviour */
    HAL_pin_set(OUT_BRAKE_L, 0);
    HAL_pin_set(OUT_BRAKE_R, 0);
    HAL_timer_reset(flash_timer, CONFIG_BRAKE_REMINDER_INTERVAL);
    flash_count = 0;
    while (!g_state.brake_requested) {

        /* do engine-off reminder flashes */
        if (g_state.engine_running) {
            HAL_timer_reset(flash_timer, CONFIG_BRAKE_REMINDER_INTERVAL);
            flash_count = 0;
        } else {
            if (HAL_timer_expired(flash_timer)) {
                switch (flash_count) {
                case 0:
                    HAL_pin_set(OUT_BRAKE_L, 1);
                    HAL_pin_set(OUT_BRAKE_R, 0);
                    flash_count = 1;
                    HAL_timer_reset(flash_timer, CONFIG_BRAKE_REMINDER_PERIOD);
                    break;
                case 1:
                    HAL_pin_set(OUT_BRAKE_L, 0);
                    HAL_pin_set(OUT_BRAKE_R, 1);
                    flash_count = 2;
                    HAL_timer_reset(flash_timer, CONFIG_BRAKE_REMINDER_PERIOD);
                    break;
                default:
                    HAL_pin_set(OUT_BRAKE_L, 0);
                    HAL_pin_set(OUT_BRAKE_R, 0);
                    flash_count = 0;
                    HAL_timer_reset(flash_timer, CONFIG_BRAKE_REMINDER_INTERVAL);
                    break;
                }
            }
        }
        pt_yield(pt);
    }

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
    g_state.brake_on = 1;
    pt_wait(pt, !g_state.brake_requested);

    /* reset idle timer */
    HAL_timer_reset(idle_timer, CONFIG_BRAKE_ATTENTION_DELAY);

    /* and reset */
    pt_reset(pt);
    return;

    pt_end(pt);
}