/*
 * Main logic for PDM
 */

#include "defs.h"

/* global status variables derived from CAN messages */
bool g_brake_applied;
bool g_can_idle;
bool g_engine_running;
struct light_status g_light_status;

PT_DEFINE(app_main)
{
    pt_begin(pt);

    for (;;) {
        /* run the brake logic */
        PT_RUN(brake);

        /* run the lights logic */
        PT_RUN(lights);

        /* run the fuel level logic */
        PT_RUN(fuel);

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
            (id == 0xa8) ||             /* brake status here */
            (id == 0x1d2)) {            /* engine speed here */
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
        g_brake_applied = (msg->data[7] > 20);
        break;

    case 0x1d2:
        g_engine_running = (msg->data[4] > 0);    /* >= 64rpm */
        break;

    case 0x21a:
        g_light_status.lights_requested = (msg->data[0] & 0x04) ? 1 : 0;
        g_light_status.rain_requested = (msg->data[0] & 0x40) ? 1 : 0;
        break;

    default:
        break;
    }
}

void
app_can_idle(bool is_idle)
{
    g_can_idle = is_idle;

    /* brake logic changes behaviour based on CAN idle state */
    PT_RESET(brake);
}
