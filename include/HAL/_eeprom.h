/** @file
 *
 * EEPROM interface.
 */

#pragma ONCE

#include <stdint.h>

/** base address of the EEPROM */
#define HAL_EEPROM_BASE     0x1400U

/**
 * Write data to the EEPROM.
 *
 * @param offset            Offset from the base of the EEPROM.
 * @param len               The number of bytes to write.
 * @param data              The data to write.
 */
extern void HAL_eeprom_write(uint16_t offset, uint8_t len, const uint8_t *data);

/**
 *  Write a byte to the EEPROM.
 *
 * @param offset            Offset from the base of the EEPROM.
 * @param data              The data to write.
 */
extern void HAL_eeprom_write8(uint16_t offset, uint8_t data);

/**
 *  Write 2 bytes to the EEPROM.
 *
 * @param offset            Offset from the base of the EEPROM.
 * @param data              The data to write.
 */
extern void HAL_eeprom_write16(uint16_t offset, uint16_t data);

/**
 *  Write 4 bytes to the EEPROM.
 *
 * @param offset            Offset from the base of the EEPROM.
 * @param data              The data to write.
 */
extern void HAL_eeprom_write32(uint16_t offset, uint32_t data);
