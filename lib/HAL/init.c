#include <stddef.h>
#include <mc9s08dz60.h>
#include <HAL/_adc.h>
#include <HAL/_bootrom.h>
#include <HAL/_can.h>
#include <HAL/_init.h>
#include <HAL/_pwm.h>
#include <HAL/_timer.h>

/* default CAN filter accepts everything */
static const HAL_can_filters_t _HAL_default_filters = {
    {
        { 0, 0 },
        { 0xffffffff, 0},
    }
};

static HAL_adc_channel_state_t _HAL_7H_adc_state[] = {
    /* AI_CS_1 */ { 10, 0, HAL_ADC_SCALE_DO_I, {0} },
    /* AI_CS_2 */ { 2,  0, HAL_ADC_SCALE_DO_I, {0} },
    /* AI_CS_3 */ { 11, 0, HAL_ADC_SCALE_DO_I, {0} },
    /* AI_CS_4 */ { 12, 0, HAL_ADC_SCALE_DO_I, {0} },
    /* AI_CS_5 */ { 0,  0, HAL_ADC_SCALE_DO_I, {0} },
    /* AI_CS_6 */ { 1,  0, HAL_ADC_SCALE_DO_I, {0} },
    /* AI_CS_7 */ { 8,  0, HAL_ADC_SCALE_DO_I, {0} },
    /* AI_KL15 */ { 14, 0, HAL_ADC_SCALE_KL15, {0} },
    /* AI_TEMP */ { 26, 0, HAL_ADC_SCALE_TEMP, {0} },
    /* END     */ { 0,  0, HAL_ADC_SCALE_END,  {0} },
};

void
_HAL_7H_init(void)
{
    _PTAD.Byte = 0x00;
    _PTADD.Byte = 0x28;
    _PTASE.Byte = 0xff;
    _PTAPE.Byte = 0x00;
    _PTADS.Byte = 0x00;

    _PTBD.Byte = 0x00;
    _PTBDD.Byte = 0x00;
    _PTBSE.Byte = 0xff;
    _PTBPE.Byte = 0x00;
    _PTBDS.Byte = 0x00;

    _PTCD.Byte = 0x00;
    _PTCDD.Byte = 0x00;
    _PTCSE.Byte = 0xff;
    _PTCPE.Byte = 0x00;
    _PTCDS.Byte = 0x00;

    _PTDD.Byte = 0x00;
    _PTDDD.Byte = 0xfe;
    _PTDSE.Byte = 0xff;
    _PTDPE.Byte = 0x00;
    _PTDDS.Byte = 0x00;

    _PTED.Byte = 0x00;
    _PTEDD.Byte = 0x14;
    _PTESE.Byte = 0xff;
    _PTEPE.Byte = 0x00;
    _PTEDS.Byte = 0x00;

    _PTFD.Byte = 0x05;
    _PTFDD.Byte = 0x05;
    _PTFSE.Byte = 0xff;
    _PTFPE.Byte = 0x00;
    _PTFDS.Byte = 0x00;

    _HAL_adc_init(_HAL_7H_adc_state);
    _HAL_pwm_init();
    _HAL_timer_init();
    HAL_can_configure(MRS_can_bitrate(), HAL_can_FM_2x32, &_HAL_default_filters);

    __asm CLI;
}

static HAL_adc_channel_state_t _HAL_7L_adc_state[] = {
    /* AI_KL15 */ { 14, 0, HAL_ADC_SCALE_KL15, {0} },
    /* AI_TEMP */ { 26, 0, HAL_ADC_SCALE_TEMP, {0} },
    /* END     */ { 0,  0, HAL_ADC_SCALE_END,  {0} },
};

void
_HAL_7L_init(void)
{
    _PTAD.Byte = 0x00;
    _PTADD.Byte = 0x00;
    _PTASE.Byte = 0xff;
    _PTAPE.Byte = 0x00;
    _PTADS.Byte = 0x00;

    _PTBD.Byte = 0x00;
    _PTBDD.Byte = 0x00;
    _PTBSE.Byte = 0xff;
    _PTBPE.Byte = 0x00;
    _PTBDS.Byte = 0x00;

    _PTCD.Byte = 0x00;
    _PTCDD.Byte = 0x00;
    _PTCSE.Byte = 0xff;
    _PTCPE.Byte = 0x00;
    _PTCDS.Byte = 0x00;

    _PTDD.Byte = 0x00;
    _PTDDD.Byte = 0xfe;
    _PTDSE.Byte = 0xff;
    _PTDPE.Byte = 0x00;
    _PTDDS.Byte = 0x00;

    _PTED.Byte = 0x00;
    _PTEDD.Byte = 0x14;
    _PTESE.Byte = 0xff;
    _PTEPE.Byte = 0x00;
    _PTEDS.Byte = 0x00;

    _PTFD.Byte = 0x07;
    _PTFDD.Byte = 0x07;
    _PTFSE.Byte = 0xff;
    _PTFPE.Byte = 0x00;
    _PTFDS.Byte = 0x00;

    _HAL_adc_init(_HAL_7L_adc_state);
    _HAL_pwm_init();
    _HAL_timer_init();
    HAL_can_configure(MRS_can_bitrate(), HAL_can_FM_2x32, &_HAL_default_filters);

    __asm CLI;
}

static HAL_adc_channel_state_t _HAL_7X_adc_state[] = {
    /* AI_CS_1 */ { 10, 0, HAL_ADC_SCALE_DO_I, {0} },
    /* AI_CS_2 */ { 2,  0, HAL_ADC_SCALE_DO_I, {0} },
    /* AI_CS_3 */ { 11, 0, HAL_ADC_SCALE_DO_I, {0} },
    /* AI_CS_4 */ { 12, 0, HAL_ADC_SCALE_DO_I, {0} },
    /* AI_OP_1 */ { 0,  0, HAL_ADC_SCALE_DO_V, {0} },
    /* AI_OP_2 */ { 1,  0, HAL_ADC_SCALE_DO_V, {0} },
    /* AI_OP_3 */ { 8,  0, HAL_ADC_SCALE_DO_V, {0} },
    /* AI_OP_4 */ { 9,  0, HAL_ADC_SCALE_DO_V, {0} },
    /* AI_1    */ { 13, 0, HAL_ADC_SCALE_30V,  {0} },
    /* AI_2    */ { 6,  0, HAL_ADC_SCALE_30V,  {0} },
    /* AI_3    */ { 7,  0, HAL_ADC_SCALE_30V,  {0} },
    /* AI_KL15 */ { 14, 0, HAL_ADC_SCALE_KL15, {0} },
    /* AI_TEMP */ { 26, 0, HAL_ADC_SCALE_TEMP, {0} },
    /* END     */ { 0,  0, HAL_ADC_SCALE_END,  {0} },
};
void
_HAL_7X_init(void)
{
    _PTAD.Byte = 0x00;
    _PTADD.Byte = 0x38;
    _PTASE.Byte = 0xff;
    _PTAPE.Byte = 0x00;
    _PTADS.Byte = 0x00;

    _PTBD.Byte = 0x00;
    _PTBDD.Byte = 0x00;
    _PTBSE.Byte = 0xff;
    _PTBPE.Byte = 0x00;
    _PTBDS.Byte = 0x00;

    _PTCD.Byte = 0x00;
    _PTCDD.Byte = 0x00;
    _PTCSE.Byte = 0xff;
    _PTCPE.Byte = 0x00;
    _PTCDS.Byte = 0x00;

    _PTDD.Byte = 0x00;
    _PTDDD.Byte = 0xf9;
    _PTDSE.Byte = 0xff;
    _PTDPE.Byte = 0x00;
    _PTDDS.Byte = 0x00;

    _PTED.Byte = 0x00;
    _PTEDD.Byte = 0x15;
    _PTESE.Byte = 0xff;
    _PTEPE.Byte = 0x00;
    _PTEDS.Byte = 0x00;

    _PTFD.Byte = 0x05;
    _PTFDD.Byte = 0x15;
    _PTFSE.Byte = 0xff;
    _PTFPE.Byte = 0x00;
    _PTFDS.Byte = 0x00;

    _HAL_adc_init(_HAL_7X_adc_state);
    _HAL_pwm_init();
    _HAL_timer_init();
    HAL_can_configure(MRS_can_bitrate(), HAL_can_FM_2x32, &_HAL_default_filters);

    __asm CLI;
}
