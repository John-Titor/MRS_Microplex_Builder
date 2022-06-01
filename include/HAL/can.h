/** @file
 *
 * CAN hardware abstraction.
 */

#pragma ONCE

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HAL_CAN_BR_100,     // index into the BTRx table
    HAL_CAN_BR_125,
    HAL_CAN_BR_250,
    HAL_CAN_BR_500,
    HAL_CAN_BR_1000
} HAL_CAN_bitrate;

typedef enum {
    HAL_CAN_FM_2x32,    // maps to the CANIDAC IDAMx bits
    HAL_CAN_FM_4x16,
    HAL_CAN_FM_8x8,
    HAL_CAN_FM_NONE
} HAL_CAN_filter_mode;

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
} HAL_CAN_filters;

// extended ID
#define HAL_CAN_ID_EXT(_id) ((((uint32_t)(_id) << 3) & (uint32_t)0xffe00000) |\
                             (((uint32_t)(_id) << 1) & (uint32_t)0x0007fffe) |\
                             (((uint32_t)3 << 19)))

// standard ID
#define HAL_CAN_ID(_id)     ((uint32_t)(_id) << 21)

// match an extended ID
#define HAL_CAN_ID_MATCH_EXT(_id, _msg) (_msg.id.mscan_id == HAL_CAN_ID_EXT(_id))

// match a standard ID - must ignore x bits
#define HAL_CAN_ID_MATCH(_id, _msg)     ((_msg.id.mscan_id & 0xfff80000) == HAL_CAN_ID(_id))


typedef struct {
    union {
        uint32_t    mscan_id;
        uint8_t     regs[4];
    } id;
    uint8_t         data[8];
    uint8_t         dlc;
    uint8_t         priority;
} HAL_CAN_message_t;

/**
 * Init MSCAN for the given bitrate.
 *
 * Set filter_mode to CAN_FM_NONE and pass NULL for filters
 * to receive all messages.
 */
extern void HAL_CAN_init(HAL_CAN_bitrate bitrate,
                         HAL_CAN_filter_mode filter_mode,
                         const HAL_CAN_filters *filters);

/**
 * Send a CAN message if there is buffer space available.
 *
 * Returns false if the message cannot be queued immediately.
 */
extern bool HAL_CAN_send(const HAL_CAN_message_t *msg);

/**
 * Send a CAN message; waits for buffer space.
 */
extern void HAL_CAN_send_blocking(const HAL_CAN_message_t *msg);

/**
 * Send a CAN message for debug purposes; waits for buffer space
 * and waits for it to be sent.
 *
 * Debug messages are always ordered vs. other debug messages.
 */
extern void HAL_CAN_send_debug(const HAL_CAN_message_t *msg);

/**
 * Attempt to receive a CAN message
 */
extern bool HAL_CAN_recv(HAL_CAN_message_t *msg);

/**
 * Send a character over the CAN console stream.
 */
extern void HAL_CAN_putchar(char c);
