/*
 * PWM output
 *
 * PWM output at ~4Hz..1kHz.
 *
 * The TPM block is clocked at 1MHz with a /4 divider giving a
 * basic quantum of 4us. Maximum period that can be requested
 * with an 8-bit argument is 255ms, or a count of 63750.
 *
 * The high-side switches used in the device slew at ~400V/ms @ 2A,
 * so they take about 30us to turn on or off. There is additionally
 * a delay of ~15us from being commanded on to starting to slew,
 * and ~40us from being commanded off to starting to slew, giving
 * a ~25us stretch to the on duration.
 *
 * To avoid runt cycles, any pulse duration less than 100us (high
 * or low) is converted to a steady state.
 */

#include <stdlib.h>

#include <mc9s08dz60.h>
#include <pwm.h>

static uint16_t pwm_period_cycles;

void
pwm_init(uint8_t period_ms)
{
    pwm_period_cycles = (uint16_t)period_ms * (1000 / 4);

    TPM1SC = 0;
    TPM1SC_CLKSx = 2;           // select fixed clock
    TPM1SC_PS = 2;              // /4 prescaler
    TPM1MOD = pwm_period_cycles;    // set PWM period
    TPM1CNT = 0;                // reset the period
}

void
pwm_set(uint8_t channel, uint8_t duty_cycle_percent)
{
    uint16_t channel_cycles;

    if (duty_cycle_percent == 0) {
        // off means off
        channel_cycles = 0;
    } else if (duty_cycle_percent >= 100) {
        // on means on
        channel_cycles = pwm_period_cycles;
    } else {
        // calcuate a reasonable approximation of what's been requested
        channel_cycles = (uint16_t)(((uint32_t)pwm_period_cycles * duty_cycle_percent) / 100);
        if (channel_cycles < 25) {
            // less than 100us on = off
            channel_cycles = 0;
        } else if ((channel_cycles + 25) > pwm_period_cycles) {
            // less than 100us off = on
            channel_cycles = pwm_period_cycles;
        }
    }

    // Configure the channel and load the new period.
    // Note that the hardware manages the period reload to avoid
    // glitches, so we don't have to.
    switch (channel) {
    case 0:
        TPM1C0SC = 0;
        TPM1C0SC_MS0B = 1;      // edge-aligned PWM
        TPM1C0SC_ELS0B = 1;     // high for assigned duty cycle
        TPM1C0V = channel_cycles;
        break;
    case 1:
        TPM1C1SC = 1;
        TPM1C1SC_MS1B = 1;      // edge-aligned PWM
        TPM1C1SC_ELS1B = 1;     // high for assigned duty cycle
        TPM1C1V = channel_cycles;
        break;
    case 2:
        TPM1C2SC = 2;
        TPM1C2SC_MS2B = 1;      // edge-aligned PWM
        TPM1C2SC_ELS2B = 1;     // high for assigned duty cycle
        TPM1C2V = channel_cycles;
        break;
    case 3:
        TPM1C3SC = 3;
        TPM1C3SC_MS3B = 1;      // edge-aligned PWM
        TPM1C3SC_ELS3B = 1;     // high for assigned duty cycle
        TPM1C3V = channel_cycles;
        break;
    case 4:
        TPM1C4SC = 4;
        TPM1C4SC_MS4B = 1;      // edge-aligned PWM
        TPM1C4SC_ELS4B = 1;     // high for assigned duty cycle
        TPM1C4V = channel_cycles;
        break;
    case 5:
        TPM1C5SC = 5;
        TPM1C5SC_MS5B = 1;      // edge-aligned PWM
        TPM1C5SC_ELS5B = 1;     // high for assigned duty cycle
        TPM1C5V = channel_cycles;
        break;
    }
}
