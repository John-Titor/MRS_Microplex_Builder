/** @file
 * PWM output.
 */

#pragma ONCE

#include <stdint.h>

/**
 * Initialize the PWM timer.
 *
 * @param period_ms [in]        PWM period in milliseconds.
 */
extern void HAL_pwm_init(uint8_t period_ms);

/**
 * Configure a PWM output.
 *
 * @param channel   [in]    PWM channel to configure.
 * @param duty      [in]    Active duty cycle; high for 7H/7X, low for 7L.
 */
extern void HAL_pwm_set(uint8_t channel, uint8_t duty);
