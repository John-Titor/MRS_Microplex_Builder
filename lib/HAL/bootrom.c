#include <stdbool.h>
#include <string.h>
#include <lib.h>
#include <HAL/_can.h>
#include <HAL/_bootrom.h>
#include <HAL/_eeprom.h>
#include <HAL/_reset.h>

#define _ID_MASK            (HAL_can_ID_EXT | 0x1ffffff0)  /* XXX TODO fetch from EEPROM */
#define _SCAN_RSP_ID        (HAL_can_ID_EXT | 0x1ffffff0)
#define _COMMAND_ID         (HAL_can_ID_EXT | 0x1ffffff1)
#define _RESPONSE_ID        (HAL_can_ID_EXT | 0x1ffffff2)
#define _EEPROM_READ_ID     (HAL_can_ID_EXT | 0x1ffffff4)
#define _EEPROM_WRITE_ID    (HAL_can_ID_EXT | 0x1ffffff5)

static bool     _module_selected = false;
static bool     _eeprom_write_enable = false;

typedef struct {
    uint32_t    id;
    uint8_t     len;
    uint8_t     cmd[5];
    void (* func)(const HAL_can_message_t *msg);
} _handler_t;

static void     _scan(const HAL_can_message_t *msg);
static void     _select(const HAL_can_message_t *msg);

static const _handler_t  _unselected_handlers[] = {
    { _COMMAND_ID,       2, { 0x00, 0x00},                   _scan },
    { _COMMAND_ID,       2, { 0x20, 0x10},                   _select }
};

static void     _enter_program(const HAL_can_message_t *msg);
static void     _read_eeprom(const HAL_can_message_t *msg);
static void     _write_eeprom_enable(const HAL_can_message_t *msg);
static void     _write_eeprom_disable(const HAL_can_message_t *msg);
static void     _write_eeprom(const HAL_can_message_t *msg);

static const _handler_t  _selected_handlers[] = {
    { _COMMAND_ID,       2, { 0x20, 0x00},                   _enter_program },
    { _COMMAND_ID,       2, { 0x20, 0x03},                   _read_eeprom },
    { _COMMAND_ID,       5, { 0x20, 0x11, 0xf3, 0x33, 0xaf}, _write_eeprom_enable },
    { _COMMAND_ID,       2, { 0x20, 0x02},                   _write_eeprom_disable },
    { _EEPROM_WRITE_ID,  0, { 0 },                           _write_eeprom }
};

static uint8_t
_can_try_bitrate(uint16_t setting)
{
    uint8_t rate_code = setting & 0xff;
    uint8_t check_code = setting >> 8;

    if ((rate_code ^ check_code) == 0xff) {
        return rate_code;
    }

    return 0;
}

uint8_t
MRS_can_bitrate(void)
{
    uint8_t rate;

    /* try each of the configured CAN bitrate sets */
    rate = _can_try_bitrate(MRS_parameters.BaudrateBootloader1);

    if (rate != 0) {
        return rate;
    }

    rate = _can_try_bitrate(MRS_parameters.BaudrateBootloader2);

    if (rate != 0) {
        return rate;
    }

    /* fall back to 125kbps */
    return MRS_CAN_125KBPS;
}

static bool
_dispatch_handler(const _handler_t *handler, uint8_t table_len, const HAL_can_message_t *msg)
{
    while (table_len--) {
        if ((handler->id == msg->id) &&                             /* command match */
                (handler->len <= msg->dlc) &&                       /* message is long enough */
                !memcmp(handler->cmd, msg->data, handler->len)) {   /* prefix matches */

            handler->func(msg);
            return true;
        }

        handler++;
    }

    return false;
}

bool
MRS_bootrom_filter(uint32_t id)
{
    return ((id & _ID_MASK) == _ID_MASK);
}

bool
MRS_bootrom_rx(const HAL_can_message_t *msg)
{
    if (_dispatch_handler(_unselected_handlers,
                          sizeof(_unselected_handlers) / sizeof(_handler_t),
                          msg)) {
        return true;
    }

    if (_module_selected &&
            _dispatch_handler(_selected_handlers,
                              sizeof(_selected_handlers) / sizeof(_handler_t),
                              msg)) {
        return true;
    }

    return false;
}

static void
_scan(const HAL_can_message_t *msg)
{
    uint8_t data[8] = {0};
    uint16_t bl_vers;
    (void)msg;

    /* send the scan response message */
    MRS_PARAM_READ(SerialNumber, &data[1]);
    MRS_PARAM_READ(BootloaderVersion, &bl_vers);
    data[7] = bl_vers & 0xff;                   /* only the low byte */
    HAL_can_send_blocking(_SCAN_RSP_ID, sizeof(data), &data[0]);

    _module_selected = false;
    _eeprom_write_enable = false;
}

static void
_enter_program(const HAL_can_message_t *msg)
{
    uint8_t data[8] = {0x2f, 0xff};
    (void)msg;

    /* send the 'will reset' message */
    MRS_PARAM_READ(SerialNumber, &data[2]);
    HAL_can_send_blocking(_RESPONSE_ID, sizeof(data), &data[0]);

    /* XXX set EEPROM to "boot to bootloader" mode? */

    /* reset immediately */
    HAL_reset();
}

static void
_select(const HAL_can_message_t *msg)
{
    const uint32_t msg_serial = *(const uint32_t *)(&msg->data[2]);
    uint8_t data[8] = {0x21, 0x10};

    /* verify that this message is selecting this module */
    if (msg_serial != MRS_parameters.SerialNumber) {

        /* someone else got selected, we should be quiet now */
        _module_selected = false;
        return;
    }

    /* send the 'selected' response */
    MRS_PARAM_READ(SerialNumber, &data[2]);
    HAL_can_send_blocking(_RESPONSE_ID, sizeof(data), &data[0]);

    _module_selected = true;
}

static void
_read_eeprom(const HAL_can_message_t *msg)
{
    const uint16_t param_offset = *(const uint16_t *)(&msg->data[2]);
    const uint8_t param_len = msg->data[4];
    uint8_t data[8];

    HAL_eeprom_read(param_offset, param_len, &data[0]);
    HAL_can_send_blocking(_EEPROM_READ_ID, param_len, &data[0]);
}

static void
_write_eeprom_enable(const HAL_can_message_t *msg)
{
    static const uint8_t data[5] = {0x21, 0x11, 0x01, 0x00, 0x00};

    (void)msg;

    _eeprom_write_enable = true;
    HAL_can_send_blocking(_RESPONSE_ID, sizeof(data), &data[0]);
}

static void
_write_eeprom_disable(const HAL_can_message_t *msg)
{
    static const uint8_t data[5] = {0x20, 0xF0, 0x02, 0x00, 0x00};

    (void)msg;

    _eeprom_write_enable = false;
    HAL_can_send_blocking(_RESPONSE_ID, sizeof(data), &data[0]);
}

static void
_write_eeprom(const HAL_can_message_t *msg)
{
    const uint16_t address = *(uint16_t *)(&msg->data[0]);
    const uint8_t len = msg->dlc - 2;
    const uint8_t *const src = &msg->data[2];
    uint8_t data[5] = {0x20, 0xe8, 0x0f};   // default to error

    if (_eeprom_write_enable) {

        /* Special-case CAN bitrate updates. */
        if ((address == MRS_PARAM_OFFSET(BaudrateBootloader1)) &&
                (len == 2) &&                           /* write just this value */
                ((src[0] ^ src[1]) == 0xff) &&          /* check code is OK */
                (src[1] >= MRS_CAN_1000KBPS) &&         /* value is within bounds */
                (src[1] <= MRS_CAN_125KBPS)) {

            /* update both copies */
            _HAL_eeprom_write(MRS_PARAM_OFFSET(BaudrateBootloader2),
                              MRS_PARAM_SIZE(BaudrateBootloader2),
                              src);
            _HAL_eeprom_write(MRS_PARAM_OFFSET(BaudrateBootloader1),
                              MRS_PARAM_SIZE(BaudrateBootloader1),
                              src);
            /* success */
            data[2] = 0;
        }

        /* Normal EEPROM write, cannot overwrite parameters. */
        else if (address >= 0x200) {
            HAL_eeprom_write(address, len, data);

            /* success */
            data[2] = 0;
        }
    }

    HAL_can_send_blocking(_RESPONSE_ID, sizeof(data), &data[0]);
}
