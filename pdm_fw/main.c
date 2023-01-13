/*
 * Main logic for PDM.
 */

#include "defs.h"

/* global status variables */
struct global_state g_state;

PT_DEFINE(app_main)
{
    pt_begin(pt);

    for (;;) {
        /* run the keypad handler */
        PT_RUN(blink_keypad);

        /* run the keypad logic */
        PT_RUN(keypad);

        /* run the lights logic */
        PT_RUN(lights);

        /* run the start button logic */
        PT_RUN(start);

        /* run the ISO-TP framer */
        PT_RUN(iso_tp);

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
        bk_can_filter(id) ||        /* blink keypad handler interested? */
        (id == 0xa8) ||             /* brake status here */
        (id == 0x1d2) ||            /* engine speed here */
        ((id >= 0x600) &&           /* ISO-TP frame? */
         (id < 0x6f0))) {

        return true;
    }

    return false;                   /* not interested */
}

void
app_can_receive(const HAL_can_message_t *msg)
{
    if (MRS_bootrom_rx(msg) ||      /* handled by bootrom code? */
        bk_can_receive(msg) ||      /* handled by blink keypad? */
        iso_tp_can_rx(msg)) {       /* handled by ISO-TP framer? */

        return;
    }

    switch (msg->id) {
    case 0xa8:
        g_state.brake_applied = (msg->data[7] > 20);
        break;

    case 0x1d2:
        g_state.engine_rpm = ((((uint16_t)msg->data[4] << 8) + msg->data[5]) / 4);

        switch (msg->data[0]) {
        case 255:
            g_state.selected_gear = 'P';
            break;

        case 210:
            g_state.selected_gear = 'R';
            break;

        case 180:
            g_state.selected_gear = 'N';
            break;

        case 120:
            g_state.selected_gear = 'D';
            break;

        default:
            g_state.selected_gear = '?';
            break;
        }

        break;

    default:
        break;
    }
}

void
app_can_idle(bool is_idle)
{
    (void)is_idle;
}
