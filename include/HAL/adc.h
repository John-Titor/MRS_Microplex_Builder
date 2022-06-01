/** @file
 * Analog to digital conversion.
 *
 * A simple moving-average filter is used, results are averaged over the last
 * ADC_AVG_SAMPLES.
 */

#pragma ONCE

#include <math.h>
#include <stdint.h>

#define HAL_ADC_AVG_SAMPLES 8

typedef struct {
    uint8_t     channel: 5;
    uint8_t     index: 3;
    uint16_t    scale_factor;
    uint16_t    samples[HAL_ADC_AVG_SAMPLES];
} HAL_adc_channel_state_t;

#define HAL_ADC_UNSCALED    4096

extern void     HAL_adc_init();
extern void     HAL_adc_configure(HAL_adc_channel_state_t *state);
extern uint16_t HAL_adc_result(HAL_adc_channel_state_t *state);
extern uint16_t HAL_adc_result_unscaled(HAL_adc_channel_state_t *state);
extern void     HAL_adc_configure_direct(uint8_t channel);

// these functions can be safely called from interrupt context
extern void     HAL_adc_update(HAL_adc_channel_state_t *state);
extern uint16_t HAL_adc_sample_direct(uint8_t channel);
