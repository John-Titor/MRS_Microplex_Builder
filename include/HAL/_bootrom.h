/** @file
 *
 * Implement the application side of the MRS bootROM / flasher protocol.
 *
 * Messages handled at 0x1ffffff1:
 *
 * receive              send
 * 00 00                00 id id id id st 00 vv     All-call "report your ID".
 * 20 00                2f ff id id id id 00 00     Enter program mode - sets EEPROM and resets.
 * 20 10 id id id id    21 10 id id id id 00 00     Select id id id id for subsequent operations.
 * 20 03 aa aa cc       dd ...                      EEPROM read cc (1-8) bytes from address aa aa.
 * 20 11 f3 33 af       21 11 01 00 00              EEPROM write enable
 * 20 02                20 f0 02 00 00              EEPROM write disable
 *
 * EEPROM write data (specific locations only) is handled at 0x1ffffff5:
 *
 * aa aa ...            20 e8 00 00 00              write eeprom data to aa aa
 *                      20 e8 0f 00 00              eeprom not unlocked
 *
 * There are lots of eeprom error messages, we just send the most generic one.
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
