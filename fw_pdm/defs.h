/*
 * Application definitions.
 */

#pragma ONCE

#include <HAL/7X.h>
#include <pt.h>
#include <blink_keypad.h>

/* restart hold-off delay */
#define CONFIG_RESTART_DELAY        250

/* CAN light message interval */
#define CONFIG_LIGHT_MSG_INTERVAL   250

/* BMW scanner config */
#define CONFIG_BMW_SCAN_INTERVAL    250
#define CONFIG_BMW_CAN_ID_BASE      0x700

/* status report interval */
#define CONFIG_STATUS_INTERVAL      250
#define CONFIG_STATUS_REPORT_ID     0x780

/* keypad key mapping */
#define KEY_A           0
#define KEY_B           1
#define KEY_C           2
#define KEY_D           3
#define KEY_LIGHTS      4
#define KEY_RAIN        5
#define KEY_G           6
#define KEY_START       7

/* input mapping */
#define IN_S_BLOW       IN_1

/* output mapping */
#define OUT_START       OUT_4

/* global state - must be <= 8B in size for status reporter */
struct global_state {
    uint16_t    engine_rpm;
    char        selected_gear;
    uint8_t     brake_applied : 1;
    uint8_t     lights_on : 1;
    uint8_t     rain_on : 1;
    uint8_t     can_idle : 1;
    uint8_t     keypad_active : 1;
    uint8_t     start_inhibited : 1;
    uint8_t     start_waiting : 1;
    uint8_t     starting : 1;
    uint8_t     debug_enable : 1;
};
extern struct global_state g_state;

/*
 * ISO-TP framer
 */
#define ISO_TP_SUCCESS      0
#define ISO_TP_BUSY         1
#define ISO_TP_TIMEOUT      2

extern uint8_t iso_tp_send(uint8_t len,         /* payload length in bytes */
                           uint8_t recipient,   /* recipient address 0x00... 0xfe */
                           uint16_t timeout_ms, /* send timeout */
                           const uint8_t *buf); /* payload bytes */
extern uint8_t iso_tp_recv(uint8_t len,         /* required reply length in bytes */
                           uint8_t sender,      /* sender address 0x00... 0xfe */
                           uint16_t timeout_ms, /* send timeout */
                           uint8_t *buf);       /* payload buffer */

extern uint8_t iso_tp_send_done(void);
extern uint8_t iso_tp_recv_done(void);
extern bool iso_tp_can_rx(const HAL_can_message_t *msg);

/* worker threads */
PT_DECLARE(keypad);
PT_DECLARE(lights);
PT_DECLARE(start);
PT_DECLARE(iso_tp);
PT_DECLARE(bmw);
PT_DECLARE(status);
