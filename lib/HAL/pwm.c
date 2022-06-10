#include <stdlib.h>
#include <mc9s08dz60.h>
#include <HAL/_pwm.h>

static uint16_t pwm_period_cycles;

void
_HAL_pwm_init(void)
{
    TPM1SC = 0;
    TPM1SC_CLKSx = 2;           /* select 1MHz fixed clock */
    TPM1SC_PS = 2;              /* /4 prescaler */
    HAL_pwm_set_period(8);      /* 8ms - sensible default */
}

void
HAL_pwm_set_period(uint8_t period_ms)
{
    pwm_period_cycles = (uint16_t)period_ms * (1000 / 4);

    TPM1MOD = pwm_period_cycles;/* set PWM period */
    TPM1CNT = 0;                /* reset the period */
}

void
HAL_pwm_set(uint8_t channel, uint8_t duty)
{
    uint16_t channel_cycles;

    if (duty == 0) {
        /* off means off */
        channel_cycles = 0;

    } else if (duty >= 100) {
        /* on means on */
        channel_cycles = pwm_period_cycles;

    } else {
        /* calcuate a reasonable approximation of what's been requested */
        channel_cycles = (uint16_t)(((uint32_t)pwm_period_cycles * duty) / 100);

        if (channel_cycles < 25) {
            /* less than 100us on = off */
            channel_cycles = 0;

        } else if ((channel_cycles + 25) > pwm_period_cycles) {
            /* less than 100us off = on */
            channel_cycles = pwm_period_cycles;
        }
    }

    /*
     * Configure the channel and load the new period.
     * Note that the hardware manages the period reload to avoid
     * glitches, so we don't have to.
     *
     * Always edge-aligned PWM, high for assigned duty cycle
     */
    switch (channel) {
    case 0:
        TPM1C0SC = TPM1C0SC_MS0B_MASK | TPM1C0SC_ELS0B_MASK;
        TPM1C0V = channel_cycles;
        break;

    case 1:
        TPM1C1SC = TPM1C1SC_MS1B_MASK | TPM1C1SC_ELS1B_MASK;
        TPM1C1V = channel_cycles;
        break;

    case 2:
        TPM1C2SC = TPM1C2SC_MS2B_MASK | TPM1C2SC_ELS2B_MASK;
        TPM1C2V = channel_cycles;
        break;

    case 3:
        TPM1C3SC = TPM1C3SC_MS3B_MASK | TPM1C3SC_ELS3B_MASK;
        TPM1C3V = channel_cycles;
        break;

    case 4:
        TPM1C4SC = TPM1C4SC_MS4B_MASK | TPM1C4SC_ELS4B_MASK;
        TPM1C4V = channel_cycles;
        break;

    case 5:
        TPM1C5SC = TPM1C5SC_MS5B_MASK | TPM1C5SC_ELS5B_MASK;
        TPM1C5V = channel_cycles;
        break;

    default:
        /* ignore */
        break;
    }
}
