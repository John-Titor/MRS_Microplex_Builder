/*
 * EEPROM hardware abstraction.
 */

#include <stdbool.h>
#include <stdint.h>

#include <mc9s08dz60.h>

#include <lib.h>
#include <HAL/_eeprom.h>

#define _EEPROM_BASE        0x1400U
#define _EEPROM_SIZE        0x800U
#define _EEPROM_SECTOR_SIZE 8
#define _EEPROM_PAGE_SIZE   (_EEPROM_SIZE / 2)
#define _EEPROM_KEEPOUT     0x200U

#define _CMD_BYTE_PROG      0x20
#define _CMD_SECTOR_ERASE   0x40

static uint16_t
_eeprom_pagesel(uint16_t offset)
{
    REQUIRE(offset < _EEPROM_SIZE);

    /* select the appropriate page for the desired byte */
    if (offset < _EEPROM_PAGE_SIZE) {
        FCNFG_EPGSEL = 0;
        return offset;
    } else {
        FCNFG_EPGSEL = 1;
        return offset - _EEPROM_PAGE_SIZE;
    }
}

static uint8_t
_eeprom_write_sector(uint16_t offset, uint8_t len, const uint8_t *data)
{
    const uint8_t data_offset = (offset % _EEPROM_SECTOR_SIZE);
    const uint8_t data_max = _EEPROM_SECTOR_SIZE - data_offset;
    const uint8_t data_len = ((len < data_max) ? len : data_max);
    const uint16_t sector_offset = _eeprom_pagesel(offset - data_offset);
    uint8_t buf[_EEPROM_SECTOR_SIZE];
    uint8_t i;
    bool need_write = false;

    REQUIRE(data_offset <= offset);
    REQUIRE(data_len <= len);
    REQUIRE(FSTAT_FCBEF == 0);
    REQUIRE(FSTAT_FACCERR == 0);
    REQUIRE(sector_offset < _EEPROM_SECTOR_SIZE);

    /* fill buffer with target data pattern */
    for (i = 0; i < data_offset; i++) {
        buf[i] = *(uint8_t *)(_EEPROM_BASE + sector_offset + i);
    }

    for (; i < (data_offset + data_len); i++) {
        buf[i] = data[i - data_offset];

        if (buf[i] != *(uint8_t *)(_EEPROM_BASE + sector_offset + i)) {
            need_write = true;
        }
    }

    for (; i < _EEPROM_SECTOR_SIZE; i++) {
        buf[i] = *(uint8_t *)(_EEPROM_BASE + sector_offset + i);
    }

    /* don't erase / write unless necessary */
    if (need_write) {

        /* erase operation will take ~20ms, ensure watchdog doesn't get upset */
        __RESET_WATCHDOG();

        /* erase sector */
        ENTER_CRITICAL_SECTION;
        *(uint8_t *)(_EEPROM_BASE + sector_offset) = 0;
        FCMD = _CMD_SECTOR_ERASE;
        FSTAT_FCBEF = 1;
        EXIT_CRITICAL_SECTION;

        while (FSTAT_FCCF == 0) {
            /* wait for operation to complete */
            REQUIRE(FSTAT_FACCERR == 0);
        }

        /* rewrite sector contents */
        for (i = 0; i < _EEPROM_SECTOR_SIZE; i++) {
            ENTER_CRITICAL_SECTION;
            *(uint8_t *)(_EEPROM_BASE + sector_offset + i) = buf[i];
            FCMD = _CMD_BYTE_PROG;
            FSTAT_FCBEF = 1;
            EXIT_CRITICAL_SECTION;

            while (FSTAT_FCCF == 0) {
                /* wait for operation to complete */
                REQUIRE(FSTAT_FACCERR == 0);
            }
        }
    }

    return data_len;
}

void
_HAL_eeprom_write(uint16_t offset, uint8_t len, const uint8_t *data)
{
    while (len) {
        uint8_t count = _eeprom_write_sector(offset, len, data);
        len -= count;
        data += count;
    }

    (void)_eeprom_pagesel(0);
}

void
HAL_eeprom_write(uint16_t offset, uint8_t len, const uint8_t *data)
{
    if (offset > _EEPROM_KEEPOUT) {
        _HAL_eeprom_write(offset, len, data);
    }
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

void
HAL_eeprom_read(uint16_t offset, uint8_t len, uint8_t *data)
{
    while (len--) {
        uint16_t bofs = _eeprom_pagesel(offset++);
        *data++ = *(uint8_t *)(_EEPROM_BASE + bofs);
    }

    (void)_eeprom_pagesel(0);
}

uint8_t
HAL_eeprom_read8(uint16_t offset)
{
    uint8_t res;

    HAL_eeprom_read(offset, sizeof(res), &res);
    return res;
}

uint16_t
HAL_eeprom_read16(uint16_t offset)
{
    uint16_t res;

    HAL_eeprom_read(offset, sizeof(res), (uint8_t *)&res);
    return res;
}

uint32_t
HAL_eeprom_read32(uint16_t offset)
{
    uint32_t res;

    HAL_eeprom_read(offset, sizeof(res), (uint8_t *)&res);
    return res;
}
