/*
 * Perform a periodic scan of BMW modules to fetch interesting
 * values, and re-broadcast them in messages that less
 * intelligent listeners can pick up.
 *
 * Note that parameters already being broadcast by DDE / EGS are
 * assumed to be handled directly.
 */

#include "defs.h"

#define ISO_DDE_ID  0x12
#define ISO_TOOL_ID 0xf1

#define ISO_TIMEOUT 100

/*
 * Setup request that is sent to configure the DDE.
 */
static const uint8_t dde_setup_req[] = {
    0x2c, 0x10, // read things
    // packed in CONFIG_BMW_CAN_ID_BASE
    0x07, 0x72, // air temperature at the HFM                       2B
    0x07, 0x6f, // air temperature after the charge cooler          2B
    0x04, 0x34, // exhaust gas temperature before particle filter   2B
    0x07, 0x6d, // boost pressure                                   2B
    // packed in CONFIG_BMW_CAN_ID_BASE + 1
    0x0e, 0xa6, // current gear                                     1B
    0x06, 0x07, // transmission oil temperature                     1B
    0x0a, 0x8d, // oil pressure status                              1B
};
#define DDE_SETUP_REQUEST_SIZE sizeof(dde_setup_req)

/*
 * Repeat request that causes the previous query to be repeated.
 */
static const uint8_t dde_repeat_req[] = {
    0x2c, 0x10, // read things
};
#define DDE_REPEAT_REQUEST_SIZE sizeof(dde_repeat_req)

// 11 bytes of data + 2 command status bytes
#define DDE_RESPONSE_SIZE 13

// we send raw from this buffer in groups of 8 starting at offset 2,
// so make the buffer multiple-of-8 + 2 large
static uint8_t dde_rx_buffer[18];

PT_DEFINE(bmw)
{
    static HAL_timer_t bmw_timeout;
    static bool setup_sent;

    pt_begin(pt);
    HAL_timer_register(bmw_timeout);
    setup_sent = false;

    for (;;) {
        uint8_t ret;

        // wait for the time to expire, then reset it
        // to minimise drift
        pt_wait(pt, HAL_timer_expired(bmw_timeout));
        HAL_timer_reset(bmw_timeout, CONFIG_BMW_SCAN_INTERVAL);

        // prepare to receive DDE response
        (void)iso_tp_recv(DDE_RESPONSE_SIZE,
                          ISO_DDE_ID,
                          ISO_TIMEOUT,
                          &dde_rx_buffer[0]);

        if (!setup_sent) {
            (void)iso_tp_send(DDE_SETUP_REQUEST_SIZE,
                              ISO_DDE_ID,
                              ISO_TIMEOUT,
                              &dde_setup_req[0]);
        } else {
            (void)iso_tp_send(DDE_REPEAT_REQUEST_SIZE,
                              ISO_DDE_ID,
                              ISO_TIMEOUT,
                              &dde_repeat_req[0]);
        }

        // wait for receive to either complete or time out - transmit
        // must have succeeded if it completes successfully.
        for (;;) {
            // give up cycles first
            pt_yield(pt);

            // check for completion
            ret = iso_tp_recv_done();

            if (ret == ISO_TP_SUCCESS) {
                break;
            }

            if (ret != ISO_TP_BUSY) {
                // no good, reset and start again
                pt_reset(pt);
                return;
            }
        }

        // Fix up the current gear field; if we are not in 'D',
        // it needs to reflect the selected gear. After fixup, the
        // field will be one of P, R, N, 1, 2, 3, 4, 5, 6.
        //
        if (g_state.selected_gear != 'D') {
            dde_rx_buffer[10] = g_state.selected_gear;
        } else {
            dde_rx_buffer[10] += '0';
        }

        // Echo the response buffer as a series of CAN frames with
        // IDs starting at 0x700.
        {
            static uint8_t sent;
            static uint32_t id;

            for (sent = 2, id = CONFIG_BMW_CAN_ID_BASE;
                    sent < DDE_RESPONSE_SIZE;
                    sent += 8, id++) {
                HAL_can_send_blocking(id, 8, &dde_rx_buffer[sent]);
                pt_yield(pt);
            }
        }

        setup_sent = true;
    }

    pt_end(pt);
}
