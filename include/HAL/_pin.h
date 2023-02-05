/** @file
 *
 * Module pins and GPIOs.
 */

#pragma ONCE

#include <stdbool.h>

/**
 * Generic GPIO.
 */
typedef struct {
    uint8_t     port: 3;    /**< A/B/C/D/E/F/NONE */
    uint8_t     pin: 3;     /**< 0-7 */
    uint8_t     : 2;
} HAL_GPIO_t;

/**
 * Module pin definition
 */
typedef struct {
    uint8_t     adc_v: 4;   /**< ADC index for voltage (see tables in init.c) */
    uint8_t     adc_i: 4;   /**< ADC index for current (see tables in init.c) */

    uint8_t     pwm: 3;     /**< PWM channel 0-5 */
    uint8_t     : 5;

    HAL_GPIO_t  range;      /**< range-control pin (30/10V) */
    HAL_GPIO_t  pull;       /**< 1k pull-up enable */

} HAL_pin_t;

/*
 * Module-specific pin definitions.
 */
extern const HAL_pin_t  _HAL_7H_pin[];
extern const HAL_pin_t  _HAL_7L_pin[];
extern const HAL_pin_t  _HAL_7X_pin[];

/**
 * Configure and drive a PWM pin in PWM output mode.
 *
 * @param pin           Pin to configure.
 * @param duty          Output duty cycle in percent.
 */
extern void     HAL_pin_set_duty(const HAL_pin_t *pin, uint8_t duty);

/**
 * Configure and drive a DO pin in digital output mode.
 *
 * This is a convenience wrapper equivalent to `HAL_pin_set_duty(value ? 100 : 0)`.
 *
 * @param pin           Pin to set.
 * @param value         True to drive the output, false to leave it open.
 */
extern void     HAL_pin_set(const HAL_pin_t *pin, bool value);

/**
 * Select the input range for a CAP_IV_RANGE pin.
 *
 * The pin should be allowed to settle before it is sampled.
 *
 * @param pin           Pin to configure.
 * @param high_range    Selects the range; true for 0-30V, false
 *                      for 0-10V.
 */
extern void     HAL_pin_set_range(const HAL_pin_t *pin, bool high_range);

/**
 * Enable or disable the pullup for a CAP_IV_PULL input pin.
 *
 * The pin should be allowed to settle before it is sampled.
 *
 * @param pin           Pin to configure.
 * @param enable        Selects whether the pullup is enabled.
 */
extern void     HAL_pin_set_pullup(const HAL_pin_t *pin, bool enable);

/**
 * Read the voltage on a pin.
 *
 * @param pin           Pin to read.
 * @return              Scaled pin voltage in mV.
 */
extern uint16_t HAL_pin_get_mV(const HAL_pin_t *pin);

/**
 * Read the current on an output pin.
 *
 * @param pin           Pin to read.
 * @return              Scaled pin current in mA.
 */
extern uint16_t HAL_pin_get_output_mA(const HAL_pin_t *pin);
