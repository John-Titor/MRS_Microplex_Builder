/*
 * Fuel level reporting.
 */
#include "defs.h"

static void
fuel_report_level(void)
{
    uint8_t buf[1];
    uint16_t fuel_mv = HAL_pin_get_mV(IN_FUEL_LEVEL);
    uint16_t percent = fuel_mv / 50;   /* 5000mV = 100% */

    g_state.fuel_mv = fuel_mv;

    if (percent > 100) {
        percent = 100;
    }

    buf[0] = percent & 0xff;
    (void)HAL_can_send_blocking(CONFIG_FUEL_MSG_ID, sizeof(buf), &buf[0]);
 }

PT_DEFINE(fuel)
{
    static HAL_timer_t report_delay;

    pt_begin(pt);
    HAL_timer_register(report_delay);

    for (;;) {
        pt_delay(pt, report_delay, CONFIG_FUEL_MSG_INTERVAL);
        fuel_report_level();
    }

    pt_end(pt);
}
