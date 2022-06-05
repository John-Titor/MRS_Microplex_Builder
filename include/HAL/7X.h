/*
 * Hardware defines for the Microplex 7X, mostly obtained by
 * digging through the sources supplied with the awful
 * Microplex Studio app, with some reference to the (brief)
 * datasheet PDF.
 *
 * SoC:                 MC9S08DZ60
 * CAN:                 TJA1043
 * High-side drivers:   VNQ5050
 * LDO:                 MIC2951
 */

#pragma ONCE

/*
 * Pin              functions
 * ---------------------------------------------------------------------------
 * 1                ground
 * 2                AI_1, FREQ_IN
 * 3                +12V
 * 4                CAN_L
 * 5                CAN_H
 * 6                DI_KL15
 * 7                AI_3
 * 8                AI_2
 * 9                DO_HSD_4, PWM_HSD4, AI_OP_4
 * 10               DO_HSD_3, PWM_HSD3, AI_OP_3
 * 11               DO_HSD_2, PWM_HSD2, AI_OP_2
 * 12               DO_HSD_1, PWM_HSD1, AI_OP_1
 *
 * Digital inputs   port        pin
 * ---------------------------------------------------------------------------
 * DI_CAN_ERR       PORT_F.3    -   CAN transceiver error signal
 * FREQ_IN          PORT_D.2    2   Frequency input (shares pin with AI_1)
 *                                  also on TPM1C0
 *
 * Digital outputs  port        pin
 * ---------------------------------------------------------------------------
 * AI_3_PU          PORT_D.3    -   1k pullup on AI_3
 * CAN_EN           PORT_F.0    -   CAN transceiver EN
 * CAN_STB_N        PORT_F.2    -   CAN transceiver STB
 * CAN_WAKE         PORT_E.5    -   CAN transceiver WAKE
 * DO_20MA_1        PORT_D.0    -   20mA current sink mode for AI_1
 * DO_20MA_2        PORT_A.3    -   20mA current sink mode for AI_2
 * DO_30V_10V_1     PORT_F.5    -   AI_1 scale select: 0 = 0-10V, 1 = 0-30V
 * DO_30V_10V_2     PORT_E.0    -   AI_2 scale select: 0 = 0-10V, 1 = 0-30V
 * DO_30V_10V_3     PORT_A.4    -   AI_3 scale select: 0 = 0-10V, 1 = 0-30V
 * DO_HSD_1         PORT_D.4    12
 * DO_HSD_2         PORT_D.7    11
 * DO_HSD_3         PORT_D.5    10
 * DO_HSD_4         PORT_D.6    9
 * DO_HSD_SEN       PORT_A.5    -   internal current sense select (STAT_DIS?)
 * DO_POWER         PORT_E.2    -   keep-alive when KL15 is removed
 *
 * Analog inputs    ADC chan    pin
 * ---------------------------------------------------------------------------
 * AI_1             13          2   (shares pin with FREQ_IN)
 * AI_2             6           8
 * AI_3             7           7
 * AI_CS_1          10          -   DO_HSD_1 sense pin
 * AI_CS_2          2           -   DO_HSD_2 sense pin
 * AI_CS_3          11          -   DO_HSD_3 sense pin
 * AI_CS_4          12          -   DO_HSD_4 sense pin
 * AI_KL15          14          6   KL15 voltage
 * AI_OP_1          0           12  DO_HSD_1 voltage
 * AI_OP_2          1           11  DO_HSD_2 voltage
 * AI_OP_3          8           10  DO_HSD_3 voltage
 * AI_OP_4          9           9   DO_HSD_4 voltage
 * AI_TEMP          26          -   Internal temperature sensor
 *
 * PWM outputs      tpm1 chan   pin digital pin
 * ---------------------------------------------------------------------------
 * PWM_HSD1         2           12  DO_HSD_1
 * PWM_HSD2         5           11  DO_HSD_2
 * PWM_HSD3         3           10  DO_HSD_3
 * PWM_HSD4         4           9   DO_HSD_4
 *
 * PortA
 * -----
 * DI_CS_2          PORT_A.2    -   DO_HSD_2 current sense
 * DO_20MA_2        PORT_A.3    -   20mA current sink mode for AI_2
 * DO_30V_10V_3     PORT_A.4    -   AI_3 scale select: 0 = 0-10V, 1 = 0-30V
 * DO_HSD_SEN       PORT_A.5    -   internal current sense disable
 * AI_2             PORT_A.6    8   Analog input 2
 * AI_3             PORT_A.7    7   Analog input 3
 *
 * PortB
 * -----
 * DI_CS_1          PORT_B.2    -   DO_HSD_1 current sense
 * DI_CS_3          PORT_B.3    -   DO_HSD_3 current sense
 * DI_CS_4          PORT_B.4    -   DO_HSD_4 current sense
 * AI_1             PORT_B.5    2   Analog input 1
 * DI_KL15          PORT_B.6    6   KL15
 *
 * PortD
 * -----
 * DO_20MA_1        PORT_D.0    -   20mA current sink mode for AI_1
 * FREQ_IN          PORT_D.2    2   Frequency input (shares pin with AI_1)
 * AI_3_PU          PORT_D.3    -   1k pullup on AI_3
 * DO_HSD_1         PORT_D.4    12
 * DO_HSD_3         PORT_D.5    10
 * DO_HSD_4         PORT_D.6    9
 * DO_HSD_2         PORT_D.7    11
 *
 * PortE
 * -----
 * DO_30V_10V_2     PORT_E.0    -   AI_2 scale select: 0 = 0-10V, 1 = 0-30V
 * DO_POWER         PORT_E.2    -   keep-alive when KL15 is removed
 * CAN_WAKE         PORT_E.5    -   CAN transceiver WAKE
 *
 * PortF
 * -----
 * CAN_EN           PORT_F.0    -   CAN transceiver EN
 * CAN_STB_N        PORT_F.2    -   CAN transceiver STB
 * DI_CAN_ERR       PORT_F.3    -   CAN transceiver ERR
 * DO_30V_10V_1     PORT_F.5    -   AI_1 scale select: 0 = 0-10V, 1 = 0-30V
 *
 *
 * Notes:
 *  - For normal operation, set CAN_EN and CAN_STB_N high. See TJA1043
 *    datasheet for other modes and the use of the CAN_WAKE signal.
 */

#include <mc9s08dz60.h>

#include <HAL/_adc.h>
#include <HAL/_bootrom.h>
#include <HAL/_can.h>
#include <HAL/_eeprom.h>
#include <HAL/_init.h>
#include <HAL/_pin.h>
#include <HAL/_pwm.h>
#include <HAL/_timer.h>

/* GPIOs */
#define DI_CAN_ERR      _PTFD.Bits.PTFD3
#define CAN_EN          _PTFD.Bits.PTFD0
#define CAN_STB_N       _PTFD.Bits.PTFD2
#define CAN_WAKE        _PTED.Bits.PTED5
#define DO_POWER        _PTED.Bits.PTED2
#define FREQ_IN         _PTDD.Bits.PTDD2
#define DO_HSD_SEN      _PTAD.Bits.PTAD5
#define DO_20MA_1       _PTDD.Bits.PTDD0
#define DO_20MA_2       _PTAD.Bits.PTAD3

/* Module pins - see _pin.h for API */
#define OUT_1           &_HAL_7X_pin[0]
#define OUT_2           &_HAL_7X_pin[1]
#define OUT_3           &_HAL_7X_pin[2]
#define OUT_4           &_HAL_7X_pin[3]
#define IN_1            &_HAL_7X_pin[4]
#define IN_2            &_HAL_7X_pin[5]
#define IN_3            &_HAL_7X_pin[6]
#define KL15            &_HAL_7X_pin[7]

#define HAL_init        _HAL_7X_init
