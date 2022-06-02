/*
 * Handle the firmware side (read-only) of the MRS flasher protocol.
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

#include <stdbool.h>
#include <string.h>

#include <lib.h>
#include <HAL/_can.h>
#include <HAL/_bootrom.h>

#define MRS_ID_MASK                 (HAL_CAN_ID_EXT | 0x1ffffff0)  // XXX TODO fetch from EEPROM
#define MRS_SCAN_RSP_ID             (HAL_CAN_ID_EXT | 0x1ffffff0)
#define MRS_COMMAND_ID              (HAL_CAN_ID_EXT | 0x1ffffff1)
#define MRS_RESPONSE_ID             (HAL_CAN_ID_EXT | 0x1ffffff2)
#define MRS_EEPROM_READ_ID          (HAL_CAN_ID_EXT | 0x1ffffff4)
#define MRS_EEPROM_WRITE_ID         (HAL_CAN_ID_EXT | 0x1ffffff5)

#define MRS_PARAM_ADDR_SERIAL       0x04
#define MRS_PARAM_ADDR_BL_VERS      0x53
#define MRS_PARAM_CAN_RATE_1        0x5b
#define MRS_PARAM_CAN_RATE_2        0x5d

#define MRS_PARAM_BASE              0x1400U

const mrs_parameters_t  mrs_parameters @(MRS_PARAM_BASE + 2);

static bool     _mrs_module_selected = false;
static bool     _mrs_eeprom_write_enable = false;

static void     _mrs_param_copy_bytes(uint16_t param_offset, uint8_t param_len, uint8_t *dst);
static bool     _mrs_param_compare_bytes(uint16_t param_offset, uint8_t param_len, const uint8_t *ref);
static void     _mrs_param_store_bytes(uint16_t param_offset, uint8_t param_len, const uint8_t *src);

typedef struct {
    uint32_t    id;
    uint8_t     len;
    uint8_t     cmd[5];
    void        (* func)(HAL_CAN_message_t *msg);
} mrs_bootrom_handler_t;

static void     _mrs_scan(HAL_CAN_message_t *msg);
static void     _mrs_select(HAL_CAN_message_t *msg);

static const mrs_bootrom_handler_t  _unselected_handlers[] = {
    { MRS_COMMAND_ID,       2, { 0x00, 0x00},                   _mrs_scan },
    { MRS_COMMAND_ID,       2, { 0x20, 0x10},                   _mrs_select }
};

static void     _mrs_enter_program(HAL_CAN_message_t *msg);
static void     _mrs_read_eeprom(HAL_CAN_message_t *msg);
static void     _mrs_write_eeprom_enable(HAL_CAN_message_t *msg);
static void     _mrs_write_eeprom_disable(HAL_CAN_message_t *msg);
static void     _mrs_write_eeprom(HAL_CAN_message_t *msg);

static const mrs_bootrom_handler_t  _selected_handlers[] = {
    { MRS_COMMAND_ID,       2, { 0x20, 0x00},                   _mrs_enter_program },
    { MRS_COMMAND_ID,       2, { 0x20, 0x03},                   _mrs_read_eeprom },
    { MRS_COMMAND_ID,       5, { 0x20, 0x11, 0xf3, 0x33, 0xaf}, _mrs_write_eeprom_enable },
    { MRS_COMMAND_ID,       2, { 0x20, 0x02},                   _mrs_write_eeprom_disable },
    { MRS_EEPROM_WRITE_ID,  0, { 0 },                           _mrs_write_eeprom }
};

static uint8_t
_mrs_can_try_bitrate(uint16_t setting)
{
    uint8_t rate_code = setting & 0xff;
    uint8_t check_code = setting >> 8;

    if ((rate_code ^ check_code) == 0xff) {
        return rate_code;
    }

    return 0;
}

uint8_t
mrs_can_bitrate(void)
{
    uint8_t rate;

    /* try each of the configured CAN bitrate sets */
    rate = _mrs_can_try_bitrate(mrs_parameters.BaudrateBootloader1);
    if (rate != 0) {
        return rate;
    }
    rate = _mrs_can_try_bitrate(mrs_parameters.BaudrateBootloader2);
    if (rate != 0) {
        return rate;
    }

    /* fall back to 125kbps */
    return MRS_CAN_125KBPS;
}

static bool
_mrs_dispatch_handler(const mrs_bootrom_handler_t *handler, uint8_t table_len, HAL_CAN_message_t *msg)
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
mrs_bootrom_rx(HAL_CAN_message_t *msg)
{
    if (_mrs_dispatch_handler(_unselected_handlers,
                              sizeof(_unselected_handlers) / sizeof(mrs_bootrom_handler_t),
                              msg)) {
        return true;
    }
    if (_mrs_module_selected
            && _mrs_dispatch_handler(_selected_handlers,
                                     sizeof(_selected_handlers) / sizeof(mrs_bootrom_handler_t),
                                     msg)) {
        return true;
    }

    return false;
}

static void
_mrs_scan(HAL_CAN_message_t *msg)
{
    uint8_t data[8] = {0};
    (void)msg;

    /* send the scan response message */
    _mrs_param_copy_bytes(MRS_PARAM_ADDR_SERIAL, 4, &data[1]);
    _mrs_param_copy_bytes(MRS_PARAM_ADDR_BL_VERS + 1, 1, &data[7]); /* low byte only */
    HAL_CAN_send_blocking(MRS_SCAN_RSP_ID,
                          sizeof(data),
                          &data[0]);

    _mrs_module_selected = false;
    _mrs_eeprom_write_enable = false;
}

static void
_mrs_enter_program(HAL_CAN_message_t *msg)
{
    uint8_t data[8] = {0x2f, 0xff};
    (void)msg;

    /* send the 'will reset' message */
    _mrs_param_copy_bytes(MRS_PARAM_ADDR_SERIAL, 4, &data[2]);
    HAL_CAN_send_blocking(MRS_RESPONSE_ID,
                          sizeof(data),
                          &data[0]);

    /* XXX set EEPROM to "boot to bootloader" mode? */

    /* XXX reset immediately */
    __asm DCW 0x9e00;
}

static void
_mrs_select(HAL_CAN_message_t *msg)
{
    uint8_t data[8] = {0x21, 0x10};

    /* verify that this message is selecting this module */
    if (!_mrs_param_compare_bytes(MRS_PARAM_ADDR_SERIAL, 4, &msg->data[2])) {

        /* someone else got selected, we should be quiet now */
        _mrs_module_selected = false;
        return;
    }

    /* send the 'selected' response */
    _mrs_param_copy_bytes(MRS_PARAM_ADDR_SERIAL, 4, &data[2]);
    /* note no bootloader version */
    HAL_CAN_send_blocking(MRS_RESPONSE_ID,
                          sizeof(data),
                          &data[0]);

    _mrs_module_selected = true;
}

static void
_mrs_read_eeprom(HAL_CAN_message_t *msg)
{
    const uint16_t param_offset = ((uint16_t)msg->data[2] << 8) | msg->data[3];
    const uint8_t param_len = msg->data[4];
    uint8_t data[8];

    _mrs_param_copy_bytes(param_offset, param_len, &data[0]);
    HAL_CAN_send_blocking(MRS_EEPROM_READ_ID,
                          param_len,
                          &data[0]);
}

static void
_mrs_write_eeprom_enable(HAL_CAN_message_t *msg)
{
    static const uint8_t data[5] = {0x21, 0x11, 0x01, 0x00, 0x00};

    (void)msg;

    _mrs_eeprom_write_enable = true;
    HAL_CAN_send_blocking(MRS_RESPONSE_ID,
                          sizeof(data),
                          &data[0]);
}

static void
_mrs_write_eeprom_disable(HAL_CAN_message_t *msg)
{
    static const uint8_t data[5] = {0x20, 0xF0, 0x02, 0x00, 0x00};

    (void)msg;

    _mrs_eeprom_write_enable = false;
    HAL_CAN_send_blocking(MRS_RESPONSE_ID,
                          sizeof(data),
                          &data[0]);
}

static void
_mrs_write_eeprom(HAL_CAN_message_t *msg)
{
    const uint16_t address = ((uint16_t)msg->data[0] << 8) | msg->data[1];
    const uint8_t len = msg->dlc - 2;
    const uint8_t *const src = &msg->data[2];
    uint8_t data[5] = {0x20, 0xe8, 0x0f};   // default to error

    if (_mrs_eeprom_write_enable) {

        // CAN bitrate update?
        if ((address == MRS_PARAM_CAN_RATE_1) &&    // speed 2 is auto-updated only
                (len == 2) &&                           // write just this value
                ((src[0] ^ src[1]) == 0xff) &&          // check code is OK
                (src[1] >= MRS_CAN_1000KBPS) &&         // value is within bounds
                (src[1] <= MRS_CAN_125KBPS)) {

            // write backup copy, and approve this write
            _mrs_param_store_bytes(MRS_PARAM_CAN_RATE_2, 2, src);
            data[2] = 0;
        }

        // write to "user" area of the EEPROM (first page only)
        else if ((address >= 0x200) &&
                 (address < 0x400) &&
                 ((address + len) <= 0x400)) {
            data[2] = 0;
        }
    }

    // if write was approved
    if (data[2] == 0) {
        _mrs_param_store_bytes(address, len, src);
    }

    // and send response
    HAL_CAN_send_blocking(MRS_RESPONSE_ID,
                          sizeof(data),
                          &data[0]);
}

static void
_mrs_param_copy_bytes(uint16_t param_offset, uint8_t param_len, uint8_t *dst)
{
    const uint8_t *src = (const uint8_t *)(MRS_PARAM_BASE + param_offset);

    while (param_len--) {
        *dst++ = *src++;
    }
}

static bool
_mrs_param_compare_bytes(uint16_t param_offset, uint8_t param_len, const uint8_t *ref)
{
    const uint8_t *src = (const uint8_t *)(MRS_PARAM_BASE + param_offset);

    while (param_len--) {
        if (*src++ != *ref++) {
            return false;
        }
    }

    return true;
}

static void
_mrs_param_store_bytes(uint16_t param_offset, uint8_t param_len, const uint8_t *src)
{
    while (param_len--) {
        // XXX (void)IEE1_SetByte(MRS_PARAM_BASE + param_offset++, *src++);
    }
}
