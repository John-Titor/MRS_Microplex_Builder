/** @file
 * Analog to digital conversion.
 *
 * The configured set of channels are sampled every 2ms,
 * and samples are averaged over the last ADC_AVG_SAMPLES.
 *
 * ADC scale factors
 * -----------------
 *
 * All measurements in 10-bit mode.
 *
 * Scaling is performed by taking the accumulated ADC counts
 * (sum of ADC_AVG_SAMPLES), multiplying by the scale factor
 * and then right-shifting by 12, i.e. the scale factor is a
 * 4.12 fixed-point quantity.
 *
 * To calculate the scaling factor, take mV-per-count and
 * multiply by 512.
 *
 * Current sense outputs are the same but for mA.
 *
 */

#pragma ONCE

#include <stdint.h>

/*
 * AI_1/2/3:
 * --------
 * 1K pullup mode: pullup connected to KL30, but no KL30 sampling
 *                 available, requires application-specific calibration.
 * 20mA mode: not supported
 */
#define _ADC_SCALE_FACTOR_30V   17900   /* VALIDATED @ 4.860V */
#define _ADC_SCALE_FACTOR_10V   6065    /* VALIDATED @ 4.860V */

/*
 * AI_OP_1/2/3/4:
 * -------------
 */
#define _ADC_SCALE_FACTOR_DO_V  16494   /* VALIDATED @ 11.46V */

/*
 * AI_CS_1/2/3/4:
 * -------------
 */
#define _ADC_SCALE_FACTOR_DO_I  4531    /* VALIDATED @ 1.000A */

/*
 * AI_KL15:
 * -------
 * Clamped at 11V; mostly useful to help detect input sag and
 * avoid faulting outputs when T30 is low.
 */
#define _ADC_SCALE_FACTOR_KL15  5507    /* VALIDATED @ 8.368V */

/*
 * AI_TEMP
 * -------
 * Calculated for nominal Vdd (5V)
 */
#define _ADC_SCALE_FACTOR_TEMP  610     /* XXX VALIDATE */

/* don't change this without adjusting the scaling factors above */
#define _HAL_ADC_AVG_SAMPLES 8

/* indices into the scaling factor table */
typedef enum {
    _HAL_ADC_SCALE_30V,
    _HAL_ADC_SCALE_10V,
    _HAL_ADC_SCALE_DO_V,
    _HAL_ADC_SCALE_DO_I,
    _HAL_ADC_SCALE_KL15,
    _HAL_ADC_SCALE_TEMP,
    _HAL_ADC_SCALE_END
} _HAL_adc_scale_t;

typedef struct {
    const uint8_t   channel: 5;
    uint8_t         scale: 3;
    uint16_t        samples[_HAL_ADC_AVG_SAMPLES];
} _HAL_adc_channel_state_t;

extern void     _HAL_adc_init(_HAL_adc_channel_state_t *state);
extern void     _HAL_adc_set_scale(uint8_t index, _HAL_adc_scale_t scale);

/**
 * Fetch a scaled ADC result.
 *
 * Note that channel indices are not range-checked.
 *
 * @param      index  The ADC channel index to fetch.
 *
 * @return     The scaled result for the channel.
 */
extern uint16_t HAL_adc_result(uint8_t index);

