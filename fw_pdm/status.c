/*
 * debug/status reporting
 */

#include "defs.h"

PT_DEFINE(status)
{
    static HAL_timer_t status_timeout;

    pt_begin(pt);
    HAL_timer_register(status_timeout);

    for (;;) {
        pt_wait(pt, HAL_timer_expired(status_timeout));
        HAL_timer_reset(status_timeout, CONFIG_STATUS_INTERVAL);

        /* send state info */
        HAL_can_send_blocking(CONFIG_STATUS_REPORT_ID, sizeof(g_state), (const uint8_t *)&g_state);
    }
    pt_end(pt);
}
