/*
 * MSCAN driver.
 */

#ifndef _MSCAN_H
#define _MSCAN_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    CAN_BR_100,     // index into the BTRx table
    CAN_BR_125,
    CAN_BR_250,
    CAN_BR_500,
    CAN_BR_1000
} CAN_bitrate;

typedef enum {
    CAN_FM_2x32,    // maps to the CANIDAC IDAMx bits
    CAN_FM_4x16,
    CAN_FM_8x8,
    CAN_FM_NONE
} CAN_filter_mode;

typedef union {
    struct {
        uint32_t    accept[2];
        uint32_t    mask[2];
    } filter_32;
    struct {
        uint16_t    accept[4];
        uint16_t    mask[4];
    } filter_16;
    struct {
        uint8_t     accept[8];
        uint8_t     mask[8];
    } filter_8;
} CAN_filters;

// extended ID
#define MSCAN_ID_EXT(_id)               ((((((uint32_t)(_id)) << 3) & (uint32_t)0xffe00000) \
                                         | (((uint32_t)(_id) << 1) & (uint32_t)0x0007fffe)) \
                                         | ((uint32_t)3 << 19))

// standard ID
#define MSCAN_ID(_id)                   ((uint32_t)(_id) << 21)

// match an extended ID
#define MSCAN_ID_MATCH_EXT(_id, _msg)   (_msg.id.mscan_id == MSCAN_ID_EXT(_id))

// match a standard ID - must ignore x bits
#define MSCAN_ID_MATCH(_id, _msg)       ((_msg.id.mscan_id & 0xfff80000) == MSCAN_ID(_id))


typedef struct {
    union {
        uint32_t    mscan_id;
        uint8_t     regs[4];
    } id;
    uint8_t         data[8];
    uint8_t         dlc;
    uint8_t         priority;
} CAN_message_t;

/**
 * Init MSCAN for the given bitrate.
 *
 * Set filter_mode to CAN_FM_NONE and pass NULL for filters
 * to receive all messages.
 */
extern void CAN_init(CAN_bitrate bitrate,
                     CAN_filter_mode filter_mode,
                     const CAN_filters *filters);

/**
 * Send a CAN message if there is buffer space available.
 *
 * Returns false if the message cannot be queued immediately.
 */
extern bool CAN_send(const CAN_message_t *msg);

/**
 * Send a CAN message; waits for buffer space.
 */
extern void CAN_send_blocking(const CAN_message_t *msg);

/**
 * Send a CAN message for debug purposes; waits for buffer space
 * and waits for it to be sent.
 *
 * Debug messages are always ordered vs. other debug messages.
 */
extern void CAN_send_debug(const CAN_message_t *msg);

/**
 * Attempt to receive a CAN message
 */
extern bool CAN_recv(CAN_message_t *msg);

/**
 * Send a character over the CAN console stream.
 */
extern void can_putchar(char c);

#endif // _MSCAN_H
