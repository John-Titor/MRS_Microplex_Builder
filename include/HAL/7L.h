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
// DO_EN_VM         PORT_F.1    -   LSD output enable (default on)
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

#include <HAL/_adc.h>
#include <HAL/_bootrom.h>
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
#define DO_EN_VM        _PTFD.Bits.PTFD1

// Module pins - see _pin.h for API
#define OUT_1           &_HAL_7L_pin[0]
#define OUT_2           &_HAL_7L_pin[1]
#define OUT_3           &_HAL_7L_pin[2]
#define OUT_4           &_HAL_7L_pin[3]
#define OUT_5           &_HAL_7L_pin[4]
#define OUT_6           &_HAL_7L_pin[5]
#define OUT_7           &_HAL_7L_pin[6]
#define KL15            &_HAL_7L_pin[7]

#define HAL_init        _HAL_7L_init
