/** @file
 *
 * CAN hardware abstraction.
 */

#pragma ONCE

#include <stdint.h>
#include <stdbool.h>

#include <pt.h>

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
} HAL_CAN_filters_t;

#define HAL_CAN_ID_EXT  ((uint32_t)1 << 31)

typedef struct {
    uint32_t        id;
    uint8_t         data[8];
    uint8_t         dlc;
} HAL_CAN_message_t;

extern void _HAL_CAN_listen(struct pt *pt);

/**
 * Configure CAN for the given bitrate.
 *
 * @param bitrate       CAN bitrate index, see @p _bootrom.h. Pass 0 to
 *                      keep the current bitrate.
 * @param filter_mode   One of the HAL_CAN_FM constants. HAL_CAN_FM_NONE
 *                      will keep the current filter configuration.
 * @param filters       Pointer to a HAL_CAN_filters_t structure containing
 *                      CAN filters. Note that masking 0x1ffffff* will cause
 *                      MRS bootrom support to stop working.
 *
 * CAN is initially configured to the same bitrate as the ROM, with
 * filters set to enable the reception of all messages.
 */
extern void HAL_CAN_configure(uint8_t bitrate,
                              HAL_CAN_filter_mode filter_mode,
                              const HAL_CAN_filters_t *filters);

/**
 * Send a CAN message if there is buffer space available.
 *
 * @param id        CAN ID to send.
 * @param dlc       Length of data.
 * @param data      Data buffer.
 * @return          false if the message cannot be queued immediately, true
 *                  if the message was queued.
 */
extern bool HAL_CAN_send(uint32_t id, uint8_t dlc, const uint8_t *data);

/**
 * Send a CAN message; waits for buffer space.
 *
 * @param id        CAN ID to send.
 * @param dlc       Length of data.
 * @param data      Data buffer.
 */
extern void HAL_CAN_send_blocking(uint32_t id, uint8_t dlc, const uint8_t *data);

/**
 * Send a CAN message for debug purposes; waits for buffer space
 * and waits for it to be sent.
 *
 * @param id        CAN ID to send.
 * @param dlc       Length of data.
 * @param data      Data buffer.
 */
extern void HAL_CAN_send_debug(uint32_t id, uint8_t dlc, const uint8_t *data);

/**
 * Fetch a CAN message from the receive buffer.
 *
 * @param msg   [out]   The received message.
 * @return              True if a message was received.
 */
extern bool HAL_CAN_recv(HAL_CAN_message_t *msg);

/**
 * Send a character over the CAN console stream.
 */
extern void HAL_CAN_putchar(char c);