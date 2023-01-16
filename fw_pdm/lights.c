/*
 * BMW CAN lighting message logic.
 */
#include "defs.h"

PT_DEFINE(lights)
{
    static HAL_timer_t report_timer;

    pt_begin(pt);
    HAL_timer_register(report_timer);
    HAL_timer_reset(report_timer, CONFIG_LIGHT_MSG_INTERVAL);

    for (;;) {
        pt_wait(pt, HAL_timer_expired(report_timer));
        HAL_timer_reset(report_timer, CONFIG_LIGHT_MSG_INTERVAL);
        {
            uint8_t data[3] = {
                0,
                0,
                0xf7
            };
            data[0] = ((g_state.lights_on ? 0x04 : 0) |
                       (g_state.rain_on ? 0x40 : 0));
            (void)HAL_can_send(0x21a, 3, &data[0]);
        }
    }

    pt_end(pt);
}
