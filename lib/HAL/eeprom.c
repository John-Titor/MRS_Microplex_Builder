/*
 * EEPROM write abstration.
 */

#include <HAL/_eeprom.h>

void
HAL_eeprom_write(uint16_t offset, uint8_t len, const uint8_t *data)
{
    (void)offset;
    (void)len;
    (void)data;
}

void
HAL_eeprom_write8(uint16_t offset, uint8_t data)
{
    HAL_eeprom_write(offset, 1, &data);
}

void
HAL_eeprom_write16(uint16_t offset, uint16_t data)
{
    HAL_eeprom_write(offset, 2, (const uint8_t *)&data);
}

void
HAL_eeprom_write32(uint16_t offset, uint32_t data)
{
    HAL_eeprom_write(offset, 4, (const uint8_t *)&data);
}
