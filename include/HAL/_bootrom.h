/** @file
 *
 * Interface to the MRS boot ROM and flash protocol.
 */

#pragma ONCE

#include <stdbool.h>
#include <stdint.h>

#include <HAL/_can.h>

/**
 * Get the CAN bitrate from EEPROM.
 */
extern uint8_t          MRS_can_bitrate(void);

/**
 * MRS CAN flash protocl filter.
 */
extern bool             MRS_bootrom_filter(uint32_t id);

/**
 * MRS CAN flash protocol handler.
 */
extern bool             MRS_bootrom_rx(const HAL_can_message_t *msg);
