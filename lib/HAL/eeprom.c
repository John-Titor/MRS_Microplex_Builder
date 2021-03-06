#include <stdbool.h>
#include <stdint.h>
#include <mc9s08dz60.h>
#include <lib.h>
#include <HAL/_eeprom.h>

#define _EEPROM_BASE        0x1400U
#define _EEPROM_SIZE        0x800U
#define _EEPROM_SECTOR_SIZE 8
#define _EEPROM_BANK_SIZE   (_EEPROM_SIZE / 2)
#define _EEPROM_KEEPOUT     0x200U

#define _CMD_BYTE_PROG      0x20
#define _CMD_SECTOR_ERASE   0x40

const MRS_parameters_t MRS_parameters;

static uint16_t
_eeprom_pagesel(uint16_t offset)
{
    REQUIRE(offset < _EEPROM_SIZE);

    /* select the appropriate page for the desired byte */
    if (offset < _EEPROM_BANK_SIZE) {
        FCNFG_EPGSEL = 0;
        return offset;
    } else {
        FCNFG_EPGSEL = 1;
        return offset - _EEPROM_BANK_SIZE;
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
    REQUIRE(data_len > 0);

    /* fill buffer from EEPROM */
    for (i = 0; i < _EEPROM_SECTOR_SIZE; i++) {
        buf[i] = *(uint8_t *)(_EEPROM_BASE + sector_offset + i);
    }

    /* compare & overwrite buffer with new data */
    for (i = 0; i < data_len; i++) {
        if (buf[data_offset + i] != data[i]) {
            buf[data_offset + i] = data[i];
            need_write = true;
        }
    }

    /* don't erase / write unless necessary */
    if (need_write) {

        /* erase operation will take ~20ms, ensure watchdog doesn't get upset */
        __RESET_WATCHDOG();

        REQUIRE(FSTAT_FCBEF != 0);
        REQUIRE(FSTAT_FACCERR == 0);

        /* erase sector */
        ENTER_CRITICAL_SECTION;
        *(uint8_t *)(_EEPROM_BASE + sector_offset) = 0;
        FCMD = _CMD_SECTOR_ERASE;
        FSTAT_FCBEF = 1;
        EXIT_CRITICAL_SECTION;

        /* wait for operation to complete */
        while (FSTAT_FCCF == 0) {
            REQUIRE(FSTAT_FACCERR == 0);
        }

        /* rewrite sector contents */
        for (i = 0; i < _EEPROM_SECTOR_SIZE; i++) {
            REQUIRE(FSTAT_FCBEF != 0);
            REQUIRE(FSTAT_FACCERR == 0);
            ENTER_CRITICAL_SECTION;
            *(uint8_t *)(_EEPROM_BASE + sector_offset + i) = buf[i];
            FCMD = _CMD_BYTE_PROG;
            FSTAT_FCBEF = 1;
            EXIT_CRITICAL_SECTION;

            /* wait for operation to complete */
            while (FSTAT_FCCF == 0) {
                REQUIRE(FSTAT_FACCERR == 0);
            }
        }
    }

    return data_len;
}

void
_HAL_eeprom_write(uint16_t offset, uint8_t len, const uint8_t *data)
{
    /*
     * Even though this is the 'private' interface, refuse to overwrite
     * any of the immutable or bootloader-owned parameters.
     */
    REQUIRE(offset >= MRS_PARAM_OFFSET(BaudrateBootloader1));

    /* Write sector-at-a-time. */
    while (len) {
        uint8_t count = _eeprom_write_sector(offset, len, data);
        len -= count;
        data += count;
    }

    /* Leave page 0 selected so that direct parameter access works. */
    (void)_eeprom_pagesel(0);
}

void
_HAL_eeprom_write_str(uint16_t offset, uint8_t len, const char *str)
{
    char buf[30];   /* large enough for largest string parameter */
    uint8_t i;

    REQUIRE(len <= sizeof(buf));

    for (i = 0; i < len; i++) {
        if (*str != 0) {
            buf[i] = *str++;
        } else {
            buf[i] = '\0';
        }
    }

    _HAL_eeprom_write(offset, len, buf);
}

void
HAL_eeprom_write(uint16_t offset, uint8_t len, const uint8_t *data)
{
    REQUIRE(offset > _EEPROM_KEEPOUT);
    _HAL_eeprom_write(offset, len, data);
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

    /* Leave page 0 selected so that direct parameter access works. */
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
