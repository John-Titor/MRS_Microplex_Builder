/*
 * ADC
 */

#include <mc9s08dz60.h>

#include <lib.h>
#include <HAL/_adc.h>

void
_HAL_adc_init(void)
{
    ADCCFG_ADICLK = 0;  // bus clock (20MHz)
    ADCCFG_MODE = 2;    // 10-bit mode
    ADCCFG_ADLSMP = 1;  // long sample time
    ADCCFG_ADIV = 2;    // /4 -> 5MHz ADCCLK

    // configure for manual conversion trigger
    ADCSC2 = 0;
}

void
HAL_adc_configure_direct(uint8_t channel)
{
    if (channel < 8) {
        APCTL1 |= (1 << channel);

    } else if (channel < 16) {
        APCTL2 |= (1 << (channel - 8));

    } else if (channel < 24) {
        APCTL3 |= (1 << (channel - 16));

    } else {
        // channels above 24 are not assigned to pins
    }
}

void
HAL_adc_configure(HAL_adc_channel_state_t *state)
{
    uint8_t i;

    HAL_adc_configure_direct(state->channel);

    // prime the channel with samples
    for (i = 0; i < HAL_ADC_AVG_SAMPLES; i++) {
        HAL_adc_update(state);
    }
}

uint16_t
HAL_adc_result(HAL_adc_channel_state_t *state)
{
    uint8_t i;
    uint32_t b;
    uint16_t accum = 0;

    // interrupt-safe loop accumulates samples
    for (i = 0; i < HAL_ADC_AVG_SAMPLES; i++) {
        uint16_t v;
        ENTER_CRITICAL_SECTION;
        v = state->samples[i];
        EXIT_CRITICAL_SECTION;
        accum += v;
    }

    // fixed-point scaling means ~60us instead of ~300 using floats
    b = (uint32_t)accum * state->scale_factor;
    return (uint16_t)(b >> 12);
}

void
HAL_adc_update(HAL_adc_channel_state_t *state)
{
    ADCSC1_ADCH = state->channel;

    // wait for completion
    while (!ADCSC1_COCO) {
    }

    state->samples[state->index++] = ADCR;
}

uint16_t
HAL_adc_sample_direct(uint8_t channel)
{
    ADCSC1_ADCH = channel;

    // wait for completion
    while (!ADCSC1_COCO) {
    }

    return ADCR;
}
