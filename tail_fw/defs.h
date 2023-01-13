/*
 * Application definitions.
 */

#pragma ONCE

#include <HAL/7X.h>
#include <pt.h>

/* Brake light attention config */
#define CONFIG_BRAKE_ATTENTION_PERIOD   250
#define CONFIG_BRAKE_ATTENTION_CYCLES   3       /* 1.5s */
#define CONFIG_BRAKE_ATTENTION_DELAY    4000    /* 4s */

/* Brake light power-on reminder config */
#define CONFIG_BRAKE_REMINDER_PERIOD    250
#define CONFIG_BRAKE_REMINDER_INTERVAL  4000

/* Brake light CAN failsafe config */
#define CONFIG_BRAKE_FAILSAFE_PERIOD    500

/* Brake brightness */
#define CONFIG_BRAKE_INTENSITY_HIGH     100
#define CONFIG_BRAKE_INTENSITY_LOW      80

/* Tail light brightness */
#define CONFIG_TAIL_INTENSITY           100

/* Rain light flash period */
#define CONFIG_RAIN_FLASH_PERIOD        125     /* 4Hz */

/* CAN fuel message */
#define CONFIG_FUEL_MSG_INTERVAL        1000    /* 1s */
#define CONFIG_FUEL_MSG_ID              0x710

/* input mapping */
#define IN_FUEL_LEVEL   IN_1

/* output mapping */
#define OUT_RAIN_LIGHT  OUT_1
#define OUT_TAIL_LIGHTS OUT_2
#define OUT_BRAKE_R     OUT_3
#define OUT_BRAKE_L     OUT_4

/* global state */
extern bool             g_brake_applied;
extern bool             g_engine_running;
extern bool             g_can_idle;

struct light_status {
    uint8_t     lights_on : 1;
    uint8_t     lights_requested : 1;
    uint8_t     rain_on : 1;
    uint8_t     rain_requested : 1;
};
extern struct light_status g_light_status;

/* worker threads */
PT_DECLARE(brake);
PT_DECLARE(lights);
PT_DECLARE(fuel);
