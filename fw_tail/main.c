/*
 * Main logic for PDM
 */

#include "defs.h"

/* global status variables derived from CAN messages */
struct global_state g_state;

PT_DEFINE(app_main)
{
    pt_begin(pt);

    /* ensure all outputs off */
    HAL_pin_set(OUT_1, false);
    HAL_pin_set(OUT_2, false);
    HAL_pin_set(OUT_3, false);
    HAL_pin_set(OUT_4, false);

    for (;;) {
        /* run the brake logic */
        PT_RUN(brake);

        /* run the lights logic */
        PT_RUN(lights);

        /* run the fuel level logic */
        PT_RUN(fuel);

        /* run the status reporter */
        PT_RUN(status);

        /* run system threads and reset watchdog */
        pt_yield(pt);
    }

    pt_end(pt);
}

void
app_init(void)
{
    HAL_init();
}

bool
app_can_filter(uint32_t id)
{
    if (MRS_bootrom_filter(id) ||   /* bootrom interested? */
        (id == 0x21a) ||            /* lighting status */
        (id == 0xa8) ||             /* brake status */
        (id == 0x1d2) ||            /* engine speed */
        (id == 0x7ff)) {            /* debug enable */
        return true;
    }

    return false;                   /* not interested */
}

void
app_can_receive(const HAL_can_message_t *msg)
{
    if (MRS_bootrom_rx(msg)) {       /* handled by bootrom code? */
        return;
    }

    switch (msg->id) {
    case 0xa8:
        g_state.brake_requested = (msg->data[7] > 20) ? 1 : 0;
        break;

    case 0x1d2:
        g_state.engine_running = (msg->data[4] > 0) ? 1 : 0;    /* >= 64rpm */
        break;

    case 0x21a:
        g_state.lights_requested = (msg->data[0] & 0x04) ? 1 : 0;
        g_state.rain_requested = (msg->data[0] & 0x40) ? 1 : 0;
        g_state.reverse_requested = (msg->data[1] & 0x01) ? 1 : 0;
        break;

    case 0x7ff:
        if ((msg->data[0] == 'd') &&
            (msg->data[1] == 'e') &&
            (msg->data[2] == 'b') &&
            (msg->data[3] == 'u') &&
            (msg->data[4] == 'g')) {
            g_state.debug_enable = msg->data[5] ? 1 : 0;
        }
        break;

    default:
        break;
    }
}

void
app_can_idle(bool is_idle)
{
    g_state.can_idle = is_idle ? 1 : 0;

    /* disable default behaviours in CAN-idle mode */
    if (is_idle && !g_state.debug_enable) {
        g_state.lights_requested = true;
        g_state.rain_requested = false;
        g_state.reverse_requested = false;

        /* brake logic changes behaviour based on CAN idle state */
        PT_RESET(brake);
    }
}
