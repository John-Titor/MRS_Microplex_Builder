/*
 * Analog / digital input logic
 */
#include "defs.h"

PT_DEFINE(input)
{
    static HAL_timer_t sample_timer;

    pt_begin(pt);
    HAL_timer_register(sample_timer);

    for(;;) {
        pt_delay(pt, sample_timer, CONFIG_INPUT_SAMPLE_INTERVAL);
        g_state.dde_switch_mv = HAL_pin_get_mV(IN_S_BLOW);
    }
    pt_end(pt);
}
