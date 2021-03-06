#include <mc9s08dz60.h>
#include <lib.h>
#include <HAL/_adc.h>
#include <HAL/_timer.h>

static const uint16_t _scale_table[] = {
    /* _HAL_ADC_SCALE_30V */    _ADC_SCALE_FACTOR_30V,
    /* _HAL_ADC_SCALE_10V */    _ADC_SCALE_FACTOR_10V,
    /* _HAL_ADC_SCALE_DO_V */   _ADC_SCALE_FACTOR_DO_V,
    /* _HAL_ADC_SCALE_DO_I */   _ADC_SCALE_FACTOR_DO_I,
    /* _HAL_ADC_SCALE_KL15 */   _ADC_SCALE_FACTOR_KL15,
    /* _HAL_ADC_SCALE_TEMP */   _ADC_SCALE_FACTOR_TEMP,
};

static _HAL_adc_channel_state_t *_state;
static uint8_t                  _sequence;
static uint8_t                  _bucket;
static HAL_timer_call_t         _call;

static void _adc_start(void);

void
_HAL_adc_init(_HAL_adc_channel_state_t *state)
{
    uint8_t i;

    _state = state;

    /*
     * configure the ADC to run as slowly as possible to leave time between
     * interrupts for other things to happen
     */
    ADCCFG_ADICLK = 1;  /* bus clock /2 (10MHz) */
    ADCCFG_ADIV = 3;    /* /8 -> 1.25MHz ADCK -> 800ns / cycle */
    ADCCFG_MODE = 2;    /* 10-bit mode */
    ADCCFG_ADLSMP = 1;  /* long sample time */

    /*
     * total conversion time: 43 ADCK cycles + 5 bus clock cycles
     * (43 * 0.8) + (5 * 0.05) = 34.65µs
     * 34µs @ 40MHz = ~1300 CPU cycles, _adc_complete() ~ 250 cycles.
     */

    /* configure for manual conversion trigger */
    ADCSC2 = 0;

    /* configure channels */
    for (i = 0; _state[i].scale != _HAL_ADC_SCALE_END; i++) {
        if (_state[i].channel < 8) {
            APCTL1 |= (1 << _state[i].channel);
        } else if (_state[i].channel < 16) {
            APCTL2 |= (1 << (_state[i].channel - 8));
        }
    }

    /* configure a periodic sample kick every 2ms */
    _call.delay_ms = 200;
    _call.period_ms = 200;
    _call.callback = _adc_start;
    HAL_timer_call_register(_call);
}

void
_HAL_adc_set_scale(uint8_t index, _HAL_adc_scale_t scale)
{
    _state[index].scale = (uint8_t)scale;
}

uint16_t
HAL_adc_result(uint8_t index)
{
    uint8_t i;
    uint32_t b;
    uint16_t accum = 0;

    /* interrupt-safe loop accumulates samples */
    for (i = 0; i < _HAL_ADC_AVG_SAMPLES; i++) {
        uint16_t v;
        ENTER_CRITICAL_SECTION;
        v = _state[index].samples[i];
        EXIT_CRITICAL_SECTION;
        accum += v;
    }

    /* fixed-point scaling for speed */
    b = (uint32_t)accum * _scale_table[_state[index].scale];
    return (uint16_t)(b >> 12);
}

static void
_adc_start(void)
{
    /* ensure that we completed the sequence last time around */
    REQUIRE(_sequence == 0);

    /* start conversion on the first channel and enable completion interrupt */
    ADCSC1_ADCH = _state[0].channel;
    ADCSC1_AIEN = 1;
}

static void
__interrupt VectorNumber_Vadc
Vadc_handler(void)
{
    /* store new sample */
    _state[_sequence].samples[_bucket] = ADCR;

    /* proceed to next channel */
    _sequence++;

    if (_state[_sequence].scale != _HAL_ADC_SCALE_END) {
        ADCSC1_ADCH = _state[_sequence].channel;
    } else {
        _sequence = 0;
        ADCSC1_AIEN = 0;

        if (++_bucket >= _HAL_ADC_AVG_SAMPLES) {
            _bucket = 0;
        }
    }
}
