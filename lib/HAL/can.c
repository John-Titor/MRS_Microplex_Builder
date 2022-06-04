/*
 * MSCAN driver.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <mc9s08dz60.h>

#include <lib.h>
#include <HAL/_can.h>
#include <HAL/_bootrom.h>

/* entries here match the MRS_CAN_*BPS constants in <HAL/_bootrom.h> */
static const uint8_t _CAN_btr_table[][2] = {
    { 0x00, 0x00 }, // -
    { 0x00, 0x05 }, // 1MHz
    { 0x00, 0x05 }, // 800kHz XXX
    { 0x00, 0x1c }, // 500kHz
    { 0x01, 0x1c }, // 250kHz
    { 0x03, 0x1c }, // 125kHz
    { 0x04, 0x1c }  // 100kHz
};

typedef enum {
    WM_NONE,
    WM_SPACE,
    WM_SENT
} _CAN_wait_mode;

static bool _CAN_send(uint32_t id,
                      uint8_t dlc,
                      const uint8_t *data,
                      _CAN_wait_mode wait);

void
HAL_CAN_configure(uint8_t bitrate,
                  HAL_CAN_filter_mode filter_mode,
                  const HAL_CAN_filters *filters)
{
    REQUIRE(bitrate <= (sizeof(_CAN_btr_table) / sizeof(_CAN_btr_table[0])));
    REQUIRE(filter_mode <= HAL_CAN_FM_NONE);

    // if rate not set, get default from ROM
    if (bitrate == 0) {
        bitrate = mrs_can_bitrate();
    }

    // set INITRQ and wait for it to be acknowledged
    CANCTL0 = CANCTL0_INITRQ_MASK;

    while (!(CANCTL1 & CANCTL1_INITAK_MASK)) {
    }

    // enable MSCAN, select external clock
    CANCTL1 = CANCTL1_CANE_MASK;

    // configure for selected bitrate
    CANBTR0 = _CAN_btr_table[bitrate][0];
    CANBTR1 = _CAN_btr_table[bitrate][1];

    // configure filters
    if (filter_mode == HAL_CAN_FM_NONE) {
        CANIDAC = CANIDAC_IDAM1_MASK; // 8-bit filters
        CANIDAR0 = 0;                 // mask result 0
        CANIDMR0 = 0;                 // look at no bits

    } else {
        CANIDAC = filter_mode << 4;
        CANIDAR0 = filters->filter_8.accept[0];
        CANIDAR1 = filters->filter_8.accept[1];
        CANIDAR2 = filters->filter_8.accept[2];
        CANIDAR3 = filters->filter_8.accept[3];
        CANIDAR4 = filters->filter_8.accept[4];
        CANIDAR5 = filters->filter_8.accept[5];
        CANIDAR6 = filters->filter_8.accept[6];
        CANIDAR7 = filters->filter_8.accept[7];
        CANIDMR0 = filters->filter_8.mask[0];
        CANIDMR1 = filters->filter_8.mask[1];
        CANIDMR2 = filters->filter_8.mask[2];
        CANIDMR3 = filters->filter_8.mask[3];
        CANIDMR4 = filters->filter_8.mask[4];
        CANIDMR5 = filters->filter_8.mask[5];
        CANIDMR6 = filters->filter_8.mask[6];
        CANIDMR7 = filters->filter_8.mask[7];
    }

    // clear INITRQ and wait for it to be acknowledged
#pragma MESSAGE DISABLE C2705
    CANCTL0 &= ~CANCTL0_INITRQ_MASK;
#pragma MESSAGE DEFAULT C2705

    while (CANCTL1 & CANCTL1_INITAK_MASK) {
    }
}

bool
HAL_CAN_send(uint32_t id, uint8_t dlc, const uint8_t *data)
{
    // return false if not possible to send immediately.
    return _CAN_send(id, dlc, data, WM_NONE);
}

void
HAL_CAN_send_blocking(uint32_t id, uint8_t dlc, const uint8_t *data)
{
    // wait for space to send message
    (void)_CAN_send(id, dlc, data, WM_SPACE);
}

void
HAL_CAN_send_debug(uint32_t id, uint8_t dlc, const uint8_t *data)
{
    // wait for space and wait for message to be sent -
    // debug messages are thus sent in the order they are
    // queued.
    (void)_CAN_send(id, dlc, data, WM_SENT);
}

static bool
_CAN_send(uint32_t id,
          uint8_t dlc,
          const uint8_t *data,
          _CAN_wait_mode wait_mode)
{
    uint8_t txe;

    REQUIRE(wait_mode <= WM_SENT);

    // wait for a buffer to be free
    for (;;) {
        txe = CANTFLG;

        if (txe != 0) {
            break;
        }

        // ... or don't
        if (wait_mode == WM_NONE) {
            return false;
        }
    }

    // select the buffer
    CANTBSEL = txe;

    // read back to work out which one we actually selected
    txe = CANTBSEL;

    // copy message to registers
    if (id & HAL_CAN_ID_EXT) {
        CANTIDR0 = (id >> 21) & 0xff;
        CANTIDR1 = (((id >> 18 & 0x07) << 5) |
                    CANTIDR1_IDE_MASK | CANTIDR1_SRR_MASK |
                    ((id >> 15) & 0x07));
        CANTIDR2 = (id >> 7) & 0xff;
        CANTIDR3 = ((id << 1) & 0xfe);
    } else {
        CANTIDR0 = (id >> 3) & 0xff;
        CANTIDR1 = (id & 0x07) << 5;
        CANTIDR3 = 0;
    }

    CANTDSR0 = data[0];
    CANTDSR1 = data[1];
    CANTDSR2 = data[2];
    CANTDSR3 = data[3];
    CANTDSR4 = data[4];
    CANTDSR5 = data[5];
    CANTDSR6 = data[6];
    CANTDSR7 = data[7];
    CANTDLR = dlc;
    CANTTBPR = 0;

    // mark the buffer as not-empty to start transmission
    CANTFLG = txe;

    // wait for message to send, or don't
    while ((wait_mode == WM_SENT) && !(CANTFLG & txe)) {
    }

    return true;
}

bool
HAL_CAN_recv(HAL_CAN_message_t *msg)
{
    REQUIRE(msg != NULL);

    // check for message in FIFO
    if (!CANRFLG_RXF) {
        return false;
    }

    // copy message from registers
    if (CANRIDR1_IDE) {
        msg->id = (((uint32_t)CANRIDR0 << 21) |
                   ((uint32_t)CANRIDR1_ID_18 << 18) |
                   ((uint32_t)CANRIDR1_ID_15 << 15) |
                   ((uint32_t)CANRIDR2 << 7) |
                   (uint32_t)CANRIDR3_ID) |
                  HAL_CAN_ID_EXT;
    } else {
        msg->id = (((uint32_t)CANRIDR0 << 3) |
                   (uint32_t)CANRIDR1_ID_18);
    }

    msg->data[0] = CANRDSR0;
    msg->data[1] = CANRDSR1;
    msg->data[2] = CANRDSR2;
    msg->data[3] = CANRDSR3;
    msg->data[4] = CANRDSR4;
    msg->data[5] = CANRDSR5;
    msg->data[6] = CANRDSR6;
    msg->data[7] = CANRDSR7;
    msg->dlc = CANRDLR;

    // mark message as consumed
    CANRFLG_RXF = 1;
    return true;
}

void
HAL_CAN_putchar(char c)
{
    static uint8_t data[8];
    static uint8_t dlc;

    data[dlc++] = c;

    // send message if full or newline
    if ((c == '\n') || (dlc == 8)) {
        HAL_CAN_send_debug(HAL_CAN_ID_EXT | 0x1ffffffeUL, dlc, &data[0]);
        dlc = 0;
    }
}
