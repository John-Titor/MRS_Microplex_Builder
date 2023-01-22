#include <stdbool.h>
#include <string.h>
#include <blink_keypad.h>
#include <lib.h>
#include <pt.h>
#include <HAL/_can.h>
#include <HAL/_timer.h>

#ifdef BK_FIXED_KEYPAD_ID
    #define _BK_DEFAULT_KEYPAD_ID  BK_FIXED_KEYPAD_ID
#else
    #define _BK_DEFAULT_KEYPAD_ID  0xff
#endif

#define BK_MAX_KEYS            12

static struct {
    uint8_t     counter;
    uint8_t     reported_state;
} _key_state[BK_MAX_KEYS];

static struct {
    uint8_t     color_a: 4;
    uint8_t     color_b: 4;
    uint8_t     pattern;
} _led_state[BK_MAX_KEYS];

static uint8_t          _num_keys;
static uint8_t          _keypad_id;
static uint8_t          _backlight_color;
static uint8_t          _backlight_intensity;
static uint8_t          _key_intensity;
static uint8_t          _update_flags;
static uint8_t          _blink_phase;

#define _UPDATE_KEYS         0x1
#define _UPDATE_INTENSITY    0x2

static void             _tick(void);
static HAL_timer_t      _idle_timer;

uint8_t
bk_num_keys(void)
{
    return _num_keys;
}

bool
bk_can_filter(uint32_t id)
{
    /* if we don't have a keypad ID the only thing we want is a boot message */
    if (_keypad_id == 0xff) {
        if ((id >= 0x700) && (id <= 0x77f)) {
            return true;
        }

        return false;
    }

    /* filter messages of interest */
    switch (id - _keypad_id) {
    case 0x580:
    case 0x180:
        return true;
    }

    return false;
}

bool
bk_can_receive(const HAL_can_message_t *msg)
{
    uint8_t     i;

    if (_keypad_id == 0xff) {
        /* Process a keypad boot message and learn the keypad ID. */
        if ((msg->id >= 0x700)
                && (msg->id <= 0x77f)
                && (msg->dlc == 1)
                && (msg->data[0] == 0)) {

            _keypad_id = msg->id & 0x7f;
            return true;
        }

        /* without a keypad ID we can't do anything else... */
        return false;
    }

    if (_num_keys == 0) {
        /*
         * Process a register continuation that we expect to be from a
         * Model ID read. Be fairly conservative.
         */
        if ((msg->id == 0x580 + _keypad_id)
                && (msg->dlc == 8)
                && (msg->data[0] == 0)
                && (msg->data[1] == 'P')
                && (msg->data[2] == 'K')
                && (msg->data[3] == 'P')) {

            uint8_t nk = msg->data[5] - '0';

            if (msg->data[4] == '2') {
                nk <<= 1;
            }

            if (nk <= BK_MAX_KEYS) {
                _num_keys = nk;
                return true;
            }
        }

        /* without a key count we can't do anything else */
        return false;
    }

    /* process a key-state message */
    if ((msg->id == (0x180 + _keypad_id))
            && (msg->dlc == 5)
            && (msg->data[2] == 0)
            && (msg->data[3] == 0)) {

        /* for each key... */
        for (i = 0; i < _num_keys; i ++) {
            /* if it is currently pressed... */
            if (msg->data[i / 8] & (1 << (i % 8))) {
                /* and it wasn't pressed... */
                if (_key_state[i].counter == 0) {
                    /* start the counter */
                    _key_state[i].counter = 1;
                }
            }
            /* if it was not pressed */
            else {
                /* reset the counter */
                _key_state[i].counter = 0;
            }
        }

        HAL_timer_reset(_idle_timer, BK_IDLE_TIMEOUT_MS);
        return true;
    }

    return false;
}

uint8_t
bk_get_event(void)
{
    uint8_t i;
    uint8_t current_state = BK_EVENT_NONE;

    /* scan key state & look for un-reported state changes */
    for (i = 0; i < _num_keys; i++) {

        /* get the most recent event that occurred for the key */
        current_state = bk_get_key_event(i);

        /* always report state transitions in order, don't skip any */
        if (current_state > _key_state[i].reported_state) {
            current_state = _key_state[i].reported_state + 0x10;
        }

        if (current_state != _key_state[i].reported_state) {
            /* report a change of state */
            break;
        }
    }

    if (i < _num_keys) {
        _key_state[i].reported_state = current_state;
        return current_state | i;
    }

    return _num_keys ? BK_EVENT_NONE : BK_EVENT_DISCONNECTED;
}

uint8_t
bk_get_key_event(uint8_t key)
{
    if (_num_keys == 0) {
        return BK_EVENT_DISCONNECTED;
    } else if (key >= _num_keys) {
        return BK_EVENT_NONE;
    }

    if (_key_state[key].counter >= BK_LONG_PRESS_3_TICKS) {
        return BK_EVENT_LONG_PRESS_3;
    } else if (_key_state[key].counter >= BK_LONG_PRESS_2_TICKS) {
        return BK_EVENT_LONG_PRESS_2;
    } else if (_key_state[key].counter >= BK_LONG_PRESS_1_TICKS) {
        return BK_EVENT_LONG_PRESS_1;
    } else if (_key_state[key].counter >= BK_SHORT_PRESS_TICKS) {
        return BK_EVENT_SHORT_PRESS;
    }

    return BK_EVENT_RELEASE;
}

void
bk_set_key_led(uint8_t key, uint8_t colors, uint8_t pattern)
{
    _led_state[key].color_a = colors & BK_COLOR_MASK;
    _led_state[key].color_b = (colors >> 4) & BK_COLOR_MASK;
    _led_state[key].pattern = pattern;
    _update_flags |= _UPDATE_KEYS;
}

void
bk_set__key_intensity(uint8_t intensity)
{
    _key_intensity = intensity & BK_MAX_INTENSITY;
    _update_flags |= _UPDATE_INTENSITY;
}

void
bk_set__backlight_color(uint8_t color)
{
    _backlight_color = color & BK_COLOR_MASK;
}

void
bk_set__backlight_intensity(uint8_t intensity)
{
    _backlight_intensity = intensity & BK_MAX_INTENSITY;
    _update_flags |= _UPDATE_INTENSITY;
}

uint8_t
bk_get_key_led(uint8_t key)
{
    const uint8_t phase_mask = 1 << _blink_phase;

    if (_led_state[key].pattern & phase_mask) {
        return _led_state[key].color_b;
    } else {
        return _led_state[key].color_a;
    }
}

static void
bk_send_led_update()
{
    const uint8_t bit_offset = (_num_keys == 12) ? 12 : (_num_keys == 10) ? 16 : 8;
    uint8_t i;
    uint8_t data[8] = {0};

    for (i = 0; i < _num_keys; i++) {
        uint8_t color = bk_get_key_led(i);
        uint8_t offset = i;

        if (color & BK_KEY_COLOR_RED) {
            data[offset / 8] |= 1 << (offset % 8);
        }

        offset += bit_offset;

        if (color & BK_KEY_COLOR_GREEN) {
            data[offset / 8] |= 1 << (offset % 8);
        }

        offset += bit_offset;

        if (color & BK_KEY_COLOR_BLUE) {
            data[offset / 8] |= 1 << (offset % 8);
        }
    }

    HAL_can_send_blocking(0x200 + _keypad_id, sizeof(data), data);
}

static void
bk_send_intensity_update()
{
    uint8_t data[8] = {0x23, 0x00, 0x65, 0x01 };

    data[4] = 0x02;
    data[5] = _key_intensity;
    HAL_can_send_blocking(0x600 + _keypad_id, sizeof(data), data);

    data[4] = 0x03;
    data[5] = _backlight_intensity;
    HAL_can_send_blocking(0x600 + _keypad_id, sizeof(data), data);

    data[4] = 0x04;
    data[5] = _backlight_color;
    HAL_can_send_blocking(0x600 + _keypad_id, sizeof(data), data);
}

PT_DEFINE(blink_keypad)
{
    static HAL_timer_t  _blink_timer;
    static HAL_timer_call_t _tick_call = {
        _tick,
        BK_TICK_PERIOD_MS,
        BK_TICK_PERIOD_MS
    };

    pt_begin(pt);

    HAL_timer_call_register(_tick_call);
    HAL_timer_register(_idle_timer);
    HAL_timer_register(_blink_timer);

    /* Zero keypad-derived state at thread start so that we can be reset. */
    _num_keys = 0;
    _keypad_id = _BK_DEFAULT_KEYPAD_ID;
    (void)memset(&_key_state, 0, sizeof(_key_state));

    for (;;) {
        /* We start here with no idea about keypad ID (unless hardcoded) or size. */

        do {
            /*
             * Give the keypad time to start talking to us after whatever we
             * just did to it.
             */
            pt_delay(pt, _blink_timer, BK_UPDATE_PERIOD_MS * 2);

            /*
             * First, try to find a keypad.
             *
             * ID may be hardcoded, or discovered by receiving a boot-up message.
             * We can't do anything else until we know what it is.
             */
            if (_keypad_id == 0xff) {

                /*
                 * Send reset-all and hope that shakes a boot-up message
                 * out of the keypad. If it has been turned off, we have
                 * to have it hardcoded.
                 */
                static const uint8_t _reset_all[] = {0x81, 0x00};
                HAL_can_send_debug(0x00, sizeof(_reset_all), _reset_all);
                continue;
            }

            /*
             * Configure the keypad the way we like it, and along the way
             * learn how many keys it has.
             *
             * The only way to do this seems to be to read the Model ID
             * and parse out the x/y dimensions from the text. Insane.
             */
            {
                static const uint8_t _init_messages[][8] = {
                    { 0x40, 0x0b, 0x10 },                       /* get model ID */
                    { 0x60 },                                   /* continue model ID */
                    { 0x2b, 0x00, 0x18, 0x05, 25, 0 },          /* 25ms announce interval */
                    { 0x2f, 0x00, 0x21, 0x00, 0x00 },           /* disable demo mode */
                    { 0x2f, 0x14, 0x20, 0x00, 0x02 },           /* fast flash only at startup */
                    { 0x2f, 0x11, 0x20, 0x00, 0x01 },           /* enable boot message */
                    { 0x2f, 0x12, 0x20, 0x00, 0x01 }            /* auto-start */
                };
                static uint8_t i;

                for (i = 0; i < (sizeof(_init_messages) / 8); i++) {
                    HAL_can_send_debug(0x600 + _keypad_id, 8, &_init_messages[i][0]);
                    pt_yield(pt);
                }
            }
        } while ((_num_keys == 0) || (_keypad_id == 0xff));

        /* we've found a keypad and we know how big it is, use it */
        /*print("keypad @ %x with %u keys", (unsigned int)_keypad_id, (unsigned int)_num_keys);*/

        for (;;) {

            /*
             * Wait until it's time to send an update; either because it's time
             * for a new animation iteration or because an LED state change was
             * requested.
             */
            pt_wait(pt, HAL_timer_expired(_blink_timer) || (_update_flags & _UPDATE_KEYS));
            _update_flags = 0;

            if (HAL_timer_expired(_blink_timer)) {
                HAL_timer_reset(_blink_timer, BK_BLINK_PERIOD_MS);
                _blink_phase = (_blink_phase + 1) & 0x7;
            }

            bk_send_led_update();

            /* this is kind of spammy, may want to be frugal with it */
            bk_send_intensity_update();

            /*
             * If the keypad disappears, reset back to the default search-for-keypad
             * state.
             */
            if (HAL_timer_expired(_idle_timer)) {
                pt_reset(pt);
                pt_yield(pt);
            }
        }
    }

    pt_end(pt);
}

void
bk_set_can_speed(uint8_t kbps)
{
    uint8_t data[8] = { 0x2f, 0x10, 0x20 };

    switch (kbps) {
    case BK_SPEED_1000:
        data[4] = 0;
        break;

    case BK_SPEED_500:
        data[4] = 2;
        break;

    case BK_SPEED_250:
        data[4] = 3;
        break;

    case BK_SPEED_125:
    default:
        data[4] = 4;
    }

    HAL_can_send_blocking(0x600 + _keypad_id, sizeof(data), data);
}

static void
_tick(void)
{
    int i;

    for (i = 0; i < _num_keys; i++) {
        if ((_key_state[i].counter > 0) && (_key_state[i].counter < 255)) {
            _key_state[i].counter++;
        }
    }
}
