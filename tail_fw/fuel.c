/*
 * Fuel level reporting.
 */
#include "defs.h"

static void
fuel_report_level(void)
{
    uint8_t buf[8];
    uint16_t mv = HAL_pin_get_mV(IN_FUEL_LEVEL);
    uint16_t percent = mv / 50;             /* 5000mV = 100% */

    if (percent > 100) {
        percent = 100;
    }

    buf[0] = percent & 0xff;
    (void)HAL_can_send(CONFIG_FUEL_MSG_ID, sizeof(buf), &buf[0]);
}

PT_DEFINE(fuel)
{
    HAL_timer_t report_delay;

    pt_begin(pt);
    HAL_timer_register(report_delay);

    for (;;) {
        pt_wait(pt, HAL_timer_expired(report_delay));
        HAL_timer_reset(report_delay, CONFIG_FUEL_MSG_INTERVAL);
        fuel_report_level();
    }

    pt_end(pt);
}