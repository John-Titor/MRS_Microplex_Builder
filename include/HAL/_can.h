/** @file
 *
 * MSCAN hardware abstraction.
 */

#pragma ONCE

#include <stdint.h>
#include <stdbool.h>
#include <pt.h>

/** CAN filter modes, maps to the CANIDAC IDAMx bits */
typedef enum {
    HAL_CAN_FM_2x32,    /**< 2x 32-bit filters */
    HAL_CAN_FM_4x16,    /**< 4x 16-bit filters */
    HAL_CAN_FM_8x8,     /**< 8x 8-bit filters */
    HAL_CAN_FM_NONE     /**< no filters (accept everything) */
} HAL_can_filter_mode;

/** Filter values */
typedef union {
    /** 32-bit filter configuration */
    struct {
        uint32_t    accept[2];  /**< 32-bit accept values */
        uint32_t    mask[2];    /**< 32-bit accept masks */
    } filter_32;
    /** 16-bit filter configuration */
    struct {
        uint16_t    accept[4];  /**< 16-bit accept values */
        uint16_t    mask[4];    /**< 16-bit accept masks */
    } filter_16;
    /** 8-bit filter configuration */
    struct {
        uint8_t     accept[8];  /**< 8-bit accept values */
        uint8_t     mask[8];    /**< 8-bit accept masks */
    } filter_8;
} HAL_can_filters_t;

/** bit indicating a CAN ID is a 29-bit extended ID */
#define HAL_CAN_ID_EXT  ((uint32_t)1 << 31)

/** CAN message structure */
typedef struct {
    uint32_t        id;         /**< message ID, 11 or 29 bits, right-aligned */
    uint8_t         data[8];    /**< message data */
    uint8_t         dlc;        /**< length of message data, 0-8 */
} HAL_can_message_t;

/** declare the CAN listener thread, run by the app framework */
PT_DECLARE(_HAL_can_listen);

/**
 * Configure CAN for the given bitrate.
 *
 * @param bitrate       CAN bitrate index, see @p _bootrom.h. Pass 0 to
 *                      keep the current bitrate.
 * @param filter_mode   One of the HAL_CAN_FM constants. HAL_CAN_FM_NONE
 *                      will keep the current filter configuration.
 * @param filters       Pointer to a HAL_can_filters_t structure containing
 *                      CAN filters. Note that masking 0x1ffffff* will cause
 *                      MRS bootrom support to stop working.
 *
 * CAN is initially configured to the same bitrate as the ROM, with
 * filters set to enable the reception of all messages.
 */
extern void HAL_can_configure(uint8_t bitrate,
                              HAL_can_filter_mode filter_mode,
                              const HAL_can_filters_t *filters);

/**
 * Send a CAN message if there is buffer space available.
 *
 * @param id        CAN ID to send.
 * @param dlc       Length of data.
 * @param data      Data buffer.
 * @return          false if the message cannot be queued immediately, true
 *                  if the message was queued.
 */
extern bool HAL_can_send(uint32_t id, uint8_t dlc, const uint8_t *data);

/**
 * Send a CAN message; waits for buffer space.
 *
 * @param id        CAN ID to send.
 * @param dlc       Length of data.
 * @param data      Data buffer.
 */
extern void HAL_can_send_blocking(uint32_t id, uint8_t dlc, const uint8_t *data);

/**
 * Send a CAN message for debug purposes; waits for buffer space
 * and waits for it to be sent.
 *
 * @param id        CAN ID to send.
 * @param dlc       Length of data.
 * @param data      Data buffer.
 */
extern void HAL_can_send_debug(uint32_t id, uint8_t dlc, const uint8_t *data);

/**
 * Fetch a CAN message from the receive buffer.
 *
 * @param msg   [out]   The received message.
 * @return              True if a message was received.
 */
extern bool HAL_can_recv(HAL_can_message_t *msg);

/**
 * Send a character over the CAN console stream.
 */
extern void HAL_can_putchar(char c);
