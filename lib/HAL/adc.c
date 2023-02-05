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

static void _adc_tick(void);

HAL_microseconds adc_channel_interval;

void
_HAL_adc_init(_HAL_adc_channel_state_t *state)
{
    uint8_t i;

    _state = state;

    /*
     * Configure the ADC to run as slowly as possible to leave time between
     * interrupts for other things to happen.
     *
     * Observed interval between interrupts ~60µs (2400 cycles).
     *
     * XXX note: not using interrupts here
     */
    ADCCFG_ADICLK = 1;  /* bus clock /2 (10MHz) */
    ADCCFG_ADIV = 3;    /* /8 -> 1.25MHz ADCK -> 800ns / cycle */
    ADCCFG_MODE = 2;    /* 10-bit mode */
    ADCCFG_ADLSMP = 1;  /* long sample time */

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

    /* configure a periodic sample kick every 1ms */
    _call.delay_ms = 1;
    _call.period_ms = 1;
    _call.callback = _adc_tick;
    HAL_timer_call_register(_call);

    /* start the first conversion */
    ADCSC1 = _state[0].channel;
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
_adc_tick(void)
{
    /* store new sample */
    _state[_sequence].samples[_bucket] = ADCR;

    /* proceed to next channel / bucket */
    if (_state[++_sequence].scale >= _HAL_ADC_SCALE_END) {
        _sequence = 0;

        if (++_bucket >= _HAL_ADC_AVG_SAMPLES) {
            _bucket = 0;
        }
    }

    /* start next conversion, no interrupt */
    ADCSC1 = _state[_sequence].channel;
}
