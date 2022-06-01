//
// Hardware defines for the Microplex 7L, mostly obtained by
// digging through the sources supplied with the awful
// Microplex Studio app, with some reference to the (brief)
// datasheet PDF.
//
// SoC:                 MC9S08DZ60
// CAN:                 TJA1043
// Low-side drivers:    Unknown (datasheet in error)
// LDO:                 MIC2951
//

#pragma ONCE

// Pin              functions
// ---------------------------------------------------------------------------
// 1                ground
// 2                DO_LSD_6, PWM_LSD_6
// 3                +12V
// 4                CAN_L
// 5                CAN_H
// 6                DI_KL15
// 7                DO_LSD_7
// 8                DO_LSD_5, PWM_LSD5
// 9                DO_LSD_4, PWM_LSD4
// 10               DO_LSD_3, PWM_LSD3
// 11               DO_LSD_2, PWM_LSD2
// 12               DO_LSD_1, PWM_LSD1
//
// Digital inputs   port        pin
// ---------------------------------------------------------------------------
// DI_CAN_ERR       PORT_F.3    -   CAN transceiver error signal
//
// Digital outputs  port        pin
// ---------------------------------------------------------------------------
// CAN_EN           PORT_F.0    -   CAN transceiver EN
// CAN_STB_N        PORT_F.2    -   CAN transceiver STB
// CAN_WAKE         PORT_E.5    -   CAN transceiver WAKE
// DO_EN_VM         PORT_F.1    -   LSD output enable
// DO_LSD_1         PORT_D.4    12
// DO_LSD_2         PORT_D.7    11
// DO_LSD_3         PORT_D.5    10
// DO_LSD_4         PORT_D.6    9
// DO_LSD_5         PORT_D.2    8
// DO_LSD_6         PORT_D.3    2
// DO_LSD_7         PORT_D.0    7
// DO_POWER         PORT_E.2    -   keep-alive when KL15 is removed
//
// Analog inputs    ADC chan    pin
// ---------------------------------------------------------------------------
// AI_KL15          14          6   KL15 voltage
// AI_TEMP          26          -   Internal temperature sensor
//
// PWM outputs      tpm1 chan   pin digital pin
// ---------------------------------------------------------------------------
// PWM_LSD1         2           12  DO_LSD_1
// PWM_LSD2         5           11  DO_LSD_2
// PWM_LSD3         3           10  DO_LSD_3
// PWM_LSD4         4           9   DO_LSD_4
// PWM_LSD5         0           8   DO_LSD_5
// PWM_LSD6         1           2   DO_LSD_6
//
// PortA
// -----
//
// PortB
// -----
// DI_KL15          PORT_B.6    6   KL15
//
// PortD
// -----
// DO_LSD_7         PORT_D.0    7
// DO_LSD_5         PORT_D.2    8
// DO_LSD_6         PORT_D.3    2
// DO_LSD_1         PORT_D.4    12
// DO_LSD_3         PORT_D.5    10
// DO_LSD_4         PORT_D.6    9
// DO_LSD_2         PORT_D.7    11
//
// PortE
// -----
// DO_POWER         PORT_E.2    -   keep-alive when KL15 is removed
// CAN_WAKE         PORT_E.5    -   CAN transceiver WAKE
//
// PortF
// -----
// CAN_EN           PORT_F.0    -   CAN transceiver EN
// CAN_STB_N        PORT_F.2    -   CAN transceiver STB
// DI_CAN_ERR       PORT_F.3    -   CAN transceiver ERR
//
//
// Notes:
//  - For normal operation, set CAN_EN and CAN_STB_N high. See TJA1043
//    datasheet for other modes and the use of the CAN_WAKE signal.

#include <mc9s08dz60.h>

#define HAL_7X          0
#define HAL_7H          0
#define HAL_7L          1

#include <HAL/adc.h>
#include <HAL/can.h>
#include <HAL/pwm.h>
#include <HAL/timer.h>

#define DI_AI_KL15      _PTBD.Bits.PTBD6
#define DI_CAN_ERR      _PTFD.Bits.PTFD3

#define CAN_EN          _PTFD.Bits.PTFD0
#define CAN_STB_N       _PTFD.Bits.PTFD2
#define CAN_WAKE        _PTED.Bits.PTED5
#define DO_EN_VM        _PTFD.Bits.PTFD1
#define DO_LSD_1        _PTDD.Bits.PTDD4
#define DO_LSD_2        _PTDD.Bits.PTDD7
#define DO_LSD_3        _PTDD.Bits.PTDD5
#define DO_LSD_4        _PTDD.Bits.PTDD6
#define DO_LSD_5        _PTDD.Bits.PTDD2
#define DO_LSD_6        _PTDD.Bits.PTDD3
#define DO_LSD_7        _PTDD.Bits.PTDD0
#define DO_POWER        _PTED.Bits.PTED2

// TPM1 channels corresponding to the LSD outputs
#define PWM_HSD_1           2
#define PWM_HSD_2           5
#define PWM_HSD_3           3
#define PWM_HSD_4           4
#define PWM_HSD_5           0
#define PWM_HSD_6           1

// ADC channel assignments
#define AI_KL15             14
#define AI_TEMP             26

// ADC scale factors
//
// Measurements in 10-bit mode.
//
// Scaling is performed by taking the accumulated ADC counts
// (sum of ADC_AVG_SAMPLES), multiplying by the scale factor
// and then right-shifting by 12, i.e. the scale factor is a
// 4.12 fixed-point quantity.
//
// To calculate the scaling factor, take mV-per-count and
// multiply by 512.
//
// Current sense outputs are the same but for mA.
//
// AI_KL15:
// -------
// Clamped at 11V; mostly useful to help detect input sag and
// avoid faulting outputs when T30 is low.

#define ADC_SCALE_FACTOR_KL15   5507    // VALIDATED @ 8.368V

// AI_TEMP
// -------
// Calculated for nominal Vdd (5V)

#define ADC_SCALE_FACTOR_TEMP   610     // XXX VALIDATE

// Initialize pins to suit the module.
//
// Note: analog inputs are configured as digital inputs
//       by default; HAL_adc_configure will claim them later.

static void
HAL_init(void)
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

    _PTFD.Byte = 0x05;
    _PTFDD.Byte = 0x07;
    _PTFSE.Byte = 0xff;
    _PTFPE.Byte = 0x00;
    _PTFDS.Byte = 0x00;

    __asm CLI;
}
