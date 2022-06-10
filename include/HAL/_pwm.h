/** @file
 * PWM output at ~4Hz..1kHz.
 *
 * The TPM block is clocked at 1MHz with a /4 divider giving a
 * basic quantum of 4us. Maximum period that can be requested
 * with an 8-bit argument is 255ms, or a count of 63750.
 *
 * The high-side switches used in the 7H/7X slew at ~400V/ms @ 2A,
 * so they take about 30us to turn on or off. There is additionally
 * a delay of ~15us from being commanded on to starting to slew,
 * and ~40us from being commanded off to starting to slew, giving
 * a ~25us stretch to the on duration.
 *
 * To avoid runt cycles, any pulse duration less than 100us (high
 * or low) is converted to a steady state.
 *
 * The 7L's drivers are not identified, but we make the assumption
 * that they're likely to be similar.
 */

#pragma ONCE

#include <stdint.h>

extern void _HAL_pwm_init(void);

/**
 * Set the PWM period.
 *
 * @param period_ms [in]        PWM period in milliseconds.
 */
extern void HAL_pwm_set_period(uint8_t period_ms);

/**
 * Configure a PWM output.
 *
 * @param channel   [in]    PWM channel to configure.
 * @param duty      [in]    Active duty cycle; high for 7H/7X, low for 7L.
 */
extern void HAL_pwm_set(uint8_t channel, uint8_t duty);
