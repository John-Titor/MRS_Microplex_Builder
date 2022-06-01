//
// Hardware defines for the Microplex 7H, mostly obtained by
// digging through the sources supplied with the awful
// Microplex Studio app, with some reference to the (brief)
// datasheet PDF.
//
// SoC:                 MC9S08DZ60
// CAN:                 TJA1043
// High-side drivers:   VNQ5050
// LDO:                 MIC2951
//

#pragma ONCE

// Pin              functions
// ---------------------------------------------------------------------------
// 1                ground
// 2                DO_HSD_6, PWM_HSD_6
// 3                +12V
// 4                CAN_L
// 5                CAN_H
// 6                DI_KL15
// 7                DO_HSD_7
// 8                DO_HSD_5, PWM_HSD5
// 9                DO_HSD_4, PWM_HSD4
// 10               DO_HSD_3, PWM_HSD3
// 11               DO_HSD_2, PWM_HSD2
// 12               DO_HSD_1, PWM_HSD1
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
// DO_HSD_1         PORT_D.4    12
// DO_HSD_2         PORT_D.7    11
// DO_HSD_3         PORT_D.5    10
// DO_HSD_4         PORT_D.6    9
// DO_HSD_5         PORT_D.2    8
// DO_HSD_6         PORT_D.3    2
// DO_HSD_7         PORT_D.0    7
// DO_HSD_SEN1      PORT_A.5    -   internal current sense select (STAT_DIS?)
// DO_HSD_SEN2      PORT_A.3    -   internal current sense select (STAT_DIS?)
// DO_POWER         PORT_E.2    -   keep-alive when KL15 is removed
//
// Analog inputs    ADC chan    pin
// ---------------------------------------------------------------------------
// AI_CS_1          10          -   DO_HSD_1 sense pin
// AI_CS_2          2           -   DO_HSD_2 sense pin
// AI_CS_3          11          -   DO_HSD_3 sense pin
// AI_CS_4          12          -   DO_HSD_4 sense pin
// AI_CS_5          0           -   DO_HSD_5 sense pin
// AI_CS_6          1           -   DO_HSD_6 sense pin
// AI_CS_7          8           -   DO_HSD_7 sense pin
// AI_KL15          14          6   KL15 voltage
// AI_TEMP          26          -   Internal temperature sensor
//
// PWM outputs      tpm1 chan   pin digital pin
// ---------------------------------------------------------------------------
// PWM_HSD1         2           12  DO_HSD_1
// PWM_HSD2         5           11  DO_HSD_2
// PWM_HSD3         3           10  DO_HSD_3
// PWM_HSD4         4           9   DO_HSD_4
// PWM_HSD5         0           8   DO_HSD_5
// PWM_HSD6         1           2   DO_HSD_6
//
// PortA
// -----
// DI_CS_5          PORT_A.0    -   DO_HSD_5 current sense
// DI_CS_6          PORT_A.1    -   DO_HSD_6 current sense
// DI_CS_2          PORT_A.2    -   DO_HSD_2 current sense
// DO_HSD_SEN2      PORT_A.3    -   internal current sense select (CS_DIS?)
// DO_HSD_SEN1      PORT_A.5    -   internal current sense select (CS_DIS?)
//
// PortB
// -----
// DI_CS_7          PORT_B.0    -   DO_HSD_7 current sense
// DI_CS_1          PORT_B.2    -   DO_HSD_1 current sense
// DI_CS_3          PORT_B.3    -   DO_HSD_3 current sense
// DI_CS_4          PORT_B.4    -   DO_HSD_4 current sense
// DI_KL15          PORT_B.6    6   KL15
//
// PortD
// -----
// DO_HSD_7         PORT_D.0    7
// DO_HSD_5         PORT_D.2    8
// DO_HSD_6         PORT_D.3    2
// DO_HSD_1         PORT_D.4    12
// DO_HSD_3         PORT_D.5    10
// DO_HSD_4         PORT_D.6    9
// DO_HSD_2         PORT_D.7    11
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
#define HAL_7H          1
#define HAL_7L          0

#include <HAL/adc.h>
#include <HAL/can.h>
#include <HAL/pwm.h>
#include <HAL/timer.h>

#define DI_AI_KL15      _PTBD.Bits.PTBD6
#define DI_CAN_ERR      _PTFD.Bits.PTFD3
#define DI_CS_1         _PTBD.Bits.PTBD2
#define DI_CS_2         _PTAD.Bits.PTAD2
#define DI_CS_3         _PTBD.Bits.PTBD3
#define DI_CS_4         _PTBD.Bits.PTBD4
#define DI_CS_5         _PTAD.Bits.PTAD0
#define DI_CS_6         _PTAD.Bits.PTAD1
#define DI_CS_7         _PTBD.Bits.PTBD0

#define CAN_EN          _PTFD.Bits.PTFD0
#define CAN_STB_N       _PTFD.Bits.PTFD2
#define CAN_WAKE        _PTED.Bits.PTED5
#define DO_HSD_1        _PTDD.Bits.PTDD4
#define DO_HSD_2        _PTDD.Bits.PTDD7
#define DO_HSD_3        _PTDD.Bits.PTDD5
#define DO_HSD_4        _PTDD.Bits.PTDD6
#define DO_HSD_5        _PTDD.Bits.PTDD2
#define DO_HSD_6        _PTDD.Bits.PTDD3
#define DO_HSD_7        _PTDD.Bits.PTDD0
#define DO_HSD_SEN1     _PTAD.Bits.PTAD3
#define DO_HSD_SEN2     _PTAD.Bits.PTAD5
#define DO_POWER        _PTED.Bits.PTED2

// TPM1 channels corresponding to the HSD outputs
#define PWM_HSD_1           2
#define PWM_HSD_2           5
#define PWM_HSD_3           3
#define PWM_HSD_4           4
#define PWM_HSD_5           0
#define PWM_HSD_6           1

// ADC channel assignments
#define AI_CS_1             10
#define AI_CS_2             2
#define AI_CS_3             11
#define AI_CS_4             12
#define AI_CS_5             0
#define AI_CS_6             1
#define AI_CS_7             8
#define AI_KL15             14
#define AI_TEMP             26

//
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
// AI_CS_1/2/3/4:
// -------------
//
#define ADC_SCALE_FACTOR_DO_I   4531    // VALIDATED @ 1.000A

// AI_KL15:
// -------
// Clamped at 11V; mostly useful to help detect input sag and
// avoid faulting outputs when T30 is low.
//

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

    __asm CLI;
}
