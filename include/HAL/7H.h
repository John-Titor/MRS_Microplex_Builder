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
// DO_HSD_SEN2      PORT_A.3    -   internal current sense disable
// DO_HSD_SEN1      PORT_A.5    -   internal current sense disable
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

#include <HAL/_adc.h>
#include <HAL/_can.h>
#include <HAL/_eeprom.h>
#include <HAL/_init.h>
#include <HAL/_pin.h>
#include <HAL/_pwm.h>
#include <HAL/_timer.h>

// GPIOs
#define DI_CAN_ERR      _PTFD.Bits.PTFD3
#define CAN_EN          _PTFD.Bits.PTFD0
#define CAN_STB_N       _PTFD.Bits.PTFD2
#define CAN_WAKE        _PTED.Bits.PTED5
#define DO_POWER        _PTED.Bits.PTED2
#define DO_HSD_SEN1     _PTAD.Bits.PTAD3
#define DO_HSD_SEN2     _PTAD.Bits.PTAD5

// Module pins - see _pin.h for API
#define OUT_1           &_HAL_7H_pin[0]
#define OUT_2           &_HAL_7H_pin[1]
#define OUT_3           &_HAL_7H_pin[2]
#define OUT_4           &_HAL_7H_pin[3]
#define OUT_5           &_HAL_7H_pin[4]
#define OUT_6           &_HAL_7H_pin[5]
#define OUT_7           &_HAL_7H_pin[6]
#define KL15            &_HAL_7H_pin[7]

#define HAL_init    _HAL_7H_init
