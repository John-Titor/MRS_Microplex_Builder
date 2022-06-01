/*
 * Analog to digital conversion.
 */

#ifndef _ADC_H
#define _ADC_H

#include <math.h>
#include <stdint.h>

/*
 * a simple moving-average filter is used, results are
 * averaged over the last ADC_AVG_SAMPLES
 */

#define ADC_AVG_SAMPLES         8

typedef struct {
    uint8_t     channel: 5;
    uint8_t     index: 3;
    uint16_t    scale_factor;
    uint16_t    samples[ADC_AVG_SAMPLES];
} adc_channel_state_t;

#define ADC_UNSCALED    4096

extern void     adc_init();
extern void     adc_configure(adc_channel_state_t *state);
extern uint16_t adc_result(adc_channel_state_t *state);
extern uint16_t adc_result_unscaled(adc_channel_state_t *state);
extern void     adc_configure_direct(uint8_t channel);

// these functions can be safely called from interrupt context
extern void     adc_update(adc_channel_state_t *state);
extern uint16_t adc_sample_direct(uint8_t channel);

#endif // _ADC_H
