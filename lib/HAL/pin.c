/*
 * Module pin abstraction.
 */

#include <stddef.h>

#include <mc9s08dz60.h>

#include <HAL/_adc.h>
#include <HAL/_pin.h>
#include <HAL/_pwm.h>

enum {
    PORT_NONE,
    PORT_A,
    PORT_B,
    PORT_C,
    PORT_D,
    PORT_E,
    PORT_F,
};

#define PWM_NONE   7
#define AI_NONE   15

const HAL_pin_t _HAL_7H_pin[] = {
    /* OUT_1 */ { AI_NONE, 0,       2        },
    /* OUT_2 */ { AI_NONE, 1,       5        },
    /* OUT_3 */ { AI_NONE, 2,       3        },
    /* OUT_4 */ { AI_NONE, 3,       4        },
    /* OUT_5 */ { AI_NONE, 4,       0        },
    /* OUT_6 */ { AI_NONE, 5,       1        },
    /* OUT_7 */ { AI_NONE, 6,       PWM_NONE },
    /* KL15  */ { 7,       AI_NONE, PWM_NONE }
};

const HAL_pin_t _HAL_7L_pin[] = {
    /* OUT_1 */ { AI_NONE, AI_NONE, 2        },
    /* OUT_2 */ { AI_NONE, AI_NONE, 5        },
    /* OUT_3 */ { AI_NONE, AI_NONE, 3        },
    /* OUT_4 */ { AI_NONE, AI_NONE, 4        },
    /* OUT_5 */ { AI_NONE, AI_NONE, 0        },
    /* OUT_6 */ { AI_NONE, AI_NONE, 1        },
    /* OUT_7 */ { AI_NONE, AI_NONE, PWM_NONE },
    /* KL15  */ { 0,       AI_NONE, PWM_NONE }
};

const HAL_pin_t _HAL_7X_pin[] = {
    /* OUT_1 */ { 4,      0,        2,        {PORT_NONE}, {PORT_NONE} },
    /* OUT_2 */ { 5,      1,        5,        {PORT_NONE}, {PORT_NONE} },
    /* OUT_3 */ { 6,      2,        3,        {PORT_NONE}, {PORT_NONE} },
    /* OUT_4 */ { 7,      3,        4,        {PORT_NONE}, {PORT_NONE} },
    /* IN_1  */ { 8,      AI_NONE,  PWM_NONE, {PORT_F, 5}, {PORT_NONE} },
    /* IN_2  */ { 9,      AI_NONE,  PWM_NONE, {PORT_E, 0}, {PORT_NONE} },
    /* IN_3  */ { 10,     AI_NONE,  PWM_NONE, {PORT_A, 4}, {PORT_A, 3} },
    /* KL15  */ { 11,     AI_NONE,  PWM_NONE, {PORT_NONE}, {PORT_NONE} }
};

static uint8_t *const _gpio_data_reg[] = {
    NULL,
    &PTAD,
    &PTBD,
    &PTCD,
    &PTDD,
    &PTED,
    &PTFD,
};

static void
_gpio_set(const HAL_GPIO_t gpio, bool value)
{
    if (gpio.port != PORT_NONE) {
        uint8_t mask = (uint8_t)1 << gpio.pin;

        if (value) {
            *_gpio_data_reg[gpio.port] |= mask;
        } else {
            *_gpio_data_reg[gpio.port] &= ~mask;
        }
    }
}

void
HAL_pin_set(const HAL_pin_t *pin, bool value)
{
    HAL_pin_set_duty(pin, value ? 100 : 0);
}

void
HAL_pin_set_duty(const HAL_pin_t *pin, uint8_t duty)
{
    HAL_pwm_set(pin->pwm, duty);
}

void
HAL_pin_set_range(const HAL_pin_t *pin, bool high_range)
{
    _gpio_set(pin->range, high_range);

    if (pin->adc_v != AI_NONE) {
        _HAL_adc_set_scale(pin->adc_v, high_range ? HAL_ADC_SCALE_30V : HAL_ADC_SCALE_10V);
    }
}

void
HAL_pin_set_pullup(const HAL_pin_t *pin, bool enable)
{
    _gpio_set(pin->pull, enable);
}

uint16_t
HAL_pin_get_mV(const HAL_pin_t *pin)
{
    if (pin->adc_v != AI_NONE) {
        return HAL_adc_result(pin->adc_v);
    } else {
        return 0;
    }
}

uint16_t
HAL_pin_get_mA(const HAL_pin_t *pin)
{
    if (pin->adc_i != AI_NONE) {
        return HAL_adc_result(pin->adc_i);
    } else {
        return 0;
    }
}
