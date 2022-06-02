/** @file
 *
 * CAN hardware abstraction.
 */

#pragma ONCE

#include <stdint.h>
#include <stdbool.h>

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

#define HAL_CAN_ID_EXT  ((uint32_t)1 << 31)

typedef struct {
    uint32_t        id;
    uint8_t         data[8];
    uint8_t         dlc;
} HAL_CAN_message_t;

/**
 * Configure CAN for the given bitrate.
 *
 * Set filter_mode to CAN_FM_NONE and pass NULL for filters
 * to receive all messages.
 */
extern void HAL_CAN_configure(uint8_t bitrate,
                              HAL_CAN_filter_mode filter_mode,
                              const HAL_CAN_filters *filters);

/**
 * Send a CAN message if there is buffer space available.
 *
 * Returns false if the message cannot be queued immediately.
 */
extern bool HAL_CAN_send(uint32_t id, uint8_t dlc, const uint8_t *data);

/**
 * Send a CAN message; waits for buffer space.
 */
extern void HAL_CAN_send_blocking(uint32_t id, uint8_t dlc, const uint8_t *data);

/**
 * Send a CAN message for debug purposes; waits for buffer space
 * and waits for it to be sent.
 *
 * Debug messages are always ordered vs. other debug messages.
 */
extern void HAL_CAN_send_debug(uint32_t id, uint8_t dlc, const uint8_t *data);

/**
 * Attempt to receive a CAN message
 */
extern bool HAL_CAN_recv(HAL_CAN_message_t *msg);

/**
 * Send a character over the CAN console stream.
 */
extern void HAL_CAN_putchar(char c);
