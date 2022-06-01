/*
 * PWM output
 */

#ifndef _PWM_H
#define _PWM_H

#include <stdint.h>

extern void pwm_init(uint8_t period_ms);
extern void pwm_set(uint8_t channel, uint8_t duty_cycle_percent);

#endif // _PWM_H
