/** @file
 *
 * Driver for the Blink Marine keypads in CanOpen mode.
 *
 */

#pragma ONCE

#include <stdbool.h>
#include <lib.h>
#include <pt.h>
#include <HAL/_can.h>

/*
 * Blink Marine keypad configuration
 */
/*#define BK_FIXED_KEYPAD_ID      0x15*/  /* assume fixed keypad ID */
#define BK_MAX_KEYS             12      /**< largest keypad supported */
#define BK_IDLE_TIMEOUT_MS      1000    /**< timeout before assuming keypad gone */
#define BK_UPDATE_PERIOD_MS     25      /**< expected keypad update interval */
#define BK_BLINK_PERIOD_MS      250     /**< time per pattern bit */
#define BK_TICK_PERIOD_MS       25      /**< interval between ticks */
#define BK_SHORT_PRESS_TICKS    2       /**< delay before registering a short press */
#define BK_LONG_PRESS_1_TICKS   20      /**< delay before registering first long press */
#define BK_LONG_PRESS_2_TICKS   60      /**< delay before registering a long press */
#define BK_LONG_PRESS_3_TICKS  120      /**< delay before registering a long press */

/*
 * Keypad event codes.
 */
#define BK_EVENT_NONE           0xff    /**< no new events */
#define BK_EVENT_DISCONNECTED   0xfe    /**< keypad not connected */
#define BK_EVENT_RELEASE        0x00    /**< key was released */
#define BK_EVENT_SHORT_PRESS    0x10    /**< key pressed for BK_SHORT_PRESS_TICKS */
#define BK_EVENT_LONG_PRESS_1   0x20    /**< key pressed for BK_LONG_PRESS_1_TICKS */
#define BK_EVENT_LONG_PRESS_2   0x30    /**< key pressed for BK_LONG_PRESS_2_TICKS */
#define BK_EVENT_LONG_PRESS_3   0x40    /**< key pressed for BK_LONG_PRESS_3_TICKS */
#define BK_EVENT_MASK           0xf0    /**< mask for event value */
#define BK_KEY_MASK             0x0f    /**< mask for key number */

/*
 * Backlight color codes.
 */
#define BK_BL_COLOR_OFF         0x0     /**< backlight off */
#define BK_BL_COLOR_RED         0x1     /**< red */
#define BK_BL_COLOR_GREEN       0x2     /**< green */
#define BK_BL_COLOR_BLUE        0x3     /**< blue */
#define BK_BL_COLOR_YELLOW      0x4     /**< yellow */
#define BK_BL_COLOR_CYAN        0x5     /**< cyan */
#define BK_BL_COLOR_MAGENTA     0x6     /**< magenta */
#define BK_BL_COLOR_WHITE       0x7     /**< white */
#define BK_BL_COLOR_AMBER       0x8     /**< amber */
#define BK_BL_COLOR_TEAL        0x9     /**< teal */

/*
 * Key color codes.
 */
#define BK_KEY_COLOR_OFF        0x0     /**< key LED off */
#define BK_KEY_COLOR_RED        0x1     /**< red */
#define BK_KEY_COLOR_GREEN      0x2     /**< green */
#define BK_KEY_COLOR_BLUE       0x4     /**< blue */
#define BK_KEY_COLOR_YELLOW     (BK_KEY_COLOR_RED | BK_KEY_COLOR_GREEN)  /**< yellow */
#define BK_KEY_COLOR_CYAN       (BK_KEY_COLOR_GREEN | BK_KEY_COLOR_BLUE) /**< cyan */
#define BK_KEY_COLOR_MAGENTA    (BK_KEY_COLOR_RED | BK_KEY_COLOR_BLUE)   /**< magenta */
#define BK_KEY_COLOR_WHITE      (BK_KEY_COLOR_RED | BK_KEY_COLOR_GREEN | BK_KEY_COLOR_BLUE)/**< white */
#define BK_COLOR_MASK           0xf     /**< key color mask */

#define BK_MAX_INTENSITY        0x3f    /**< max LED intensity */

/*
 * CAN bus speeds
 */
#define BK_SPEED_1000           0       /**< 1Mbps */
#define BK_SPEED_500            1       /**< 500kbps */
#define BK_SPEED_250            2       /**< 250kbps */
#define BK_SPEED_125            3       /**< 125kbps */

/** Keypad handler thread, must be run by app for keypad to function. */
PT_DECLARE(blink_keypad);

/**
 * Get the number of keys on the keypad.
 *
 * @return              The number of keys on the keypad. Zero
 *                      if a keypad has not been detected, or
 *                      a previously-detected keypad has disappeared.
 */
extern uint8_t bk_num_keys(void);

/**
 * Sniff a CAN ID and decide whether it is interesting to the driver.
 *
 * Should be called from @p app_can_filter for any message that might
 * be from a keypad.
 *
 * @param id            CAN message ID.
 */
extern bool bk_can_filter(uint32_t id);

/**
 * Feed a received CAN message to the keypad handler.
 *
 * Should be called from @p app_can_receive for any message that
 * might be from a keypad.
 *
 * @param msg           The CAN message buffer.
 * @returns             true if the message was consumed, false
 *                      if it was not a keypad message.
 */
extern bool bk_can_receive(const HAL_can_message_t *msg);

/**
 * Get an event from the keypad.
 *
 * @returns             Event code in the high 4 bits, key number in the low 4.
 *                      BK_EVENT_NONE if no event occurred,
 *                      BK_EVENT_DISCONNECTED if a keypad is not connected.
 */
extern uint8_t bk_get_event(void);

/**
 * Get the most recent event for a given key.
 *
 * Note that this returns the event that put the key into its current
 * state.
 *
 * @param key           Key number (0-11)
 * @returns             Event code in the high 4 bits, BK_EVENT_NONE if
 *                      the key number is not valid for the current keypad,
 *                      BK_EVENT_DISCONNECTED if a keypad is not connected.
 */
extern uint8_t bk_get_key_event(uint8_t key);

/**
 * Set a key LED.
 *
 * @param key           Key number (0-11).
 * @param colors        Colors A and B in the low and high 4 bits respectively.
 * @param pattern       Each bit corresponds to a 250ms slot in a 2s repeating cycle.
 *                      A zero bit gives color A, a 1 bit gives color b.  All-zeros
 *                      gives constant color A, etc.
 */
extern void bk_set_key_led(uint8_t key, uint8_t colors, uint8_t pattern);

/**
 * Get the instant state of a key LED.
 *
 * @param key
 * @returns             A color code indicating what the LED is doing right now.
 */
extern uint8_t bk_get_key_led(uint8_t key);

/**
 * Set key LED brightness.
 *
 * @param intensity     Key LED intensity (0-0x3f)
 */
extern void bk_set_key_intensity(uint8_t intensity);

/**
 * Set the backlight color.
 */
extern void bk_set_backlight_color(uint8_t color);

/**
 * Set the backlight brightness.
 *
 * @param intensity     Backlight intensity (0-0x3f).
 */
extern void bk_set_backlight_intensity(uint8_t intensity);

/**
 * Set the keypad CAN speed
 *
 * Most useful when changing local CAN speed configuration at the same time.
 */
extern void bk_set_can_speed(uint8_t speed);
