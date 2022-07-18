# E36 power distribution module

## system

The vehicle electronic system consists of this module, the M57 DDE, ZF 6HP transmission controller (EGS), fuel pump controller (EKPM), MK60 ABS, AEM CD-7 dash, and another MicroPlex 7X managing the tail lights.

## pinout

| pin | signal  | function | description
|-----|---------|----------|-------------
| 1   | GND	    |		   |
| 2   | S_BLOW  | IN_1     | DDE power-on signal (actuates DDE relay)
| 3   | ACC_MRS |          |
| 4   | CAN_L   |          |
| 5   | CAN_H   |          |
| 6   | T15_MRS | KL15     | Ignition-on signal
| 7   | -       | IN_2	   |
| 8   | -       | IN_3	   |
| 9   | -       | OUT_4	   |
| 10  | -       | OUT_3	   |
| 11  | -       | OUT_2	   |
| 12  | S_START | OUT_1	   | Starter relay


## functionality

### starter safety

Ensure that the starter relay can only be activated in a safe state.

Starting requires:
 - engine not running (CAN engine speed below 100 for 250ms)
 - brake pedal pressed (CAN brake signal)
 - transmission in park (CAN transmission position) (or neutral? TBD)
 - DME powered on (S_BLOW low)
 - start button pressed for 250ms (keypad)

Behaviours:
 - keypad button red when start not possible and engine not running
 - keypad button green when start possible
 - keypad button flash while attempting to start
 - keypad button off when engine running

### light switching

Enable switching of lights from the keypad.

Behaviours:
 - tail lights bit in BMW lighting CAN message toggles with button press
 - rain lights bit (reverse) in BMW lighting CAN message toggles with button press
 - buttons white when lights off, blue when lights on

### DDE scanning

Extract parameters from the DDE and reformat as CAN messages that can be handled by the dash.

Parameters are not scaled, just extracted and forwarded as-is.

| pid        | size | description
|------------|------|-----------------
| 0x07, 0x72 |    2 | air temperature at the HFM                       
| 0x07, 0x6f |    2 | air temperature after the charge cooler          
| 0x04, 0x34 |    2 | exhaust gas temperature before particle filter   
| 0x07, 0x6d |    2 | boost pressure                                   
| 0x0e, 0xa6 |    1 | current gear (1)
| 0x06, 0x07 |    1 | transmission oil temperature (1)
| 0x0a, 0x8d |    1 | oil pressure status

(1) since these are transmission parameters, they are probably on the wire in messages
    from the EGS that have not yet been decoded.

PID reading is performed using ISO-TP encapsulated BMW (KWP-2000?) commands.

## future possibilities

 - headlights
 - wipers
 - fan control
 - shirt cooler control (with CAN:RS485 bridge)

## CAN messages of interest

| id       | details 
|----------|-----------------
| a8/168   | byte 8 = brake status; low values (<20) brake released, high values (>20) brake applied
| aa/170   | bytes 2/3 = throttle position 0-65535
|          | bytes 4/5 = engine speed * 4
| 1d0/464  | byte 0 = coolant temperature in °C, 0 = -48°C
| 1d2/466  | byte 0 = selected gear, 225 = P, 210 = R, 180 = N, 120 = D
| 592/1426 | byte 3, bit 0 = check engine

BO_ 168 EngineAndBrake: 8 DME
 SG_ BrakePressed : 61|1@0+ (1,0) [0|1] "" XXX
 SG_ Brake_active2 : 62|1@0+ (1,0) [0|1] "" XXX
 SG_ EngineTorque : 8|16@1- (0.03125,0) [-1024|1023] "" XXX
 SG_ EngineTorqueWoInterv : 24|16@1- (0.03125,0) [-1024|1023.96875] "" XXX

BO_ 170 AccPedal: 8 DME
 SG_ KickDownPressed : 53|1@0+ (1,0) [0|3] "" XXX
 SG_ CruisePedalActive : 54|1@0+ (1,0) [0|1] "" XXX
 SG_ CruisePedalInactive : 55|1@0+ (1,0) [0|1] "" XXX
 SG_ ThrottlelPressed : 50|1@0+ (1,0) [0|1] "" XXX
 SG_ AcceleratorPedalPressed : 52|1@0+ (1,0) [0|7] "" XXX
 SG_ AcceleratorPedalPercentage : 16|16@1+ (0.04,0) [0|100] "" XXX
 SG_ Counter_170 : 8|4@1+ (1,0) [0|15] "" XXX
 SG_ EngineSpeed : 32|16@1+ (0.25,0) [0|8000] "rpm" XXX
 SG_ Checksum_170 : 0|8@1- (1,0) [0|65535] "" XXX

BO_ 464 EngineData: 8 DME
 SG_ Counter_464 : 23|8@0+ (1,0) [0|65535] "" XXX

**seems to be missing coolant temp**

BO_ 466 TransmissionDataDisplay: 8 EGS
 SG_ ShiftLeverMode : 32|2@1+ (1,0) [0|3] "" XXX
 SG_ GearAct : 12|4@1+ (1,-4) [0|15] "" XXX
 SG_ Counter_466 : 28|4@1+ (1,0) [0|14] "" XXX
 SG_ ShiftLeverPosition : 0|4@1+ (1,0) [0|8] "" XXX
 SG_ xFF : 40|8@1+ (1,0) [0|255] "" XXX
 SG_ ShiftLeverPositionXOR : 4|4@1+ (1,0) [0|0] "" Vector__XXX
 SG_ SportButtonState : 26|1@1+ (1,0) [0|1] "" XXX

## DDE / EGS parameter extraction

Firmware uses the BMW parameter reading protocol to query DDE and EGS parameters, then reports these in a fashion that the PDM08 can handle.

The BMW parameter protocol is essentially their K-line protocol encapsulated in ISO-TP https://en.wikipedia.org/wiki/ISO_15765-2 frames with extended addressing. To keep things simple, we only query a single PID per message.

### DDE query frame format

XXX is this correct? is there a more compact query format?

CAN ID | Byte 0 | Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7
------ | ------ | ------ | ------ | ------ | ------ | ------ | ------ | ------
0x6f1  | ECU ID | length | 0x2c   | 0x10   | PID0   | PID1   | 0x00   | 0x00

ECU ID : 0x12 = DDE, 0x18 = EGS
length : 5..7 depending on the length of PID data
PIDx   : PID bytes (see below)

### DDE response frame format

CAN ID | Byte 0 | Byte 1 | Byte 2 | Byte 3 | Byte 4..7
------ | ------ | ------ | ------ | ------ | ---------
ECU ID | 0xf1   | length | 0x6c   | 0x10   | Data 0..3

ECU ID : 0x612 = DDE, 0x618 = EGS
length : 5..7 depending on the length of the data
Data   : parameter data, unused bytes are 0xff

### DDE PIDs

PID0 | PID1 | response | description
---- | ---- | -------- | -----------
0x03 | 0x85 | TH TL    | fuel temperature in °C = (TH:TL / 100) - 55
0x04 | 0x1b | TH TL    | exhaust temperature in °C = (TH:TL / 32) - 50
0x07 | 0x6f | TH TL    | manifold air temperature in °C = (TH:TL / 100) - 100
0x07 | 0x6d | PH PL    | PH:PL = manifold pressure in mBar
0x0a | 0x8d | SS       | oil pressure status, 00 = OK, 01 = low oil pressure
0x0e | 0xa6 | GG       | current gear = GG
0x10 | 0x06 | SS       | MIL indicator status

### EGS query frame format

CAN ID | Byte 0 | Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7
------ | ------ | ------ | ------ | ------ | ------ | ------ | ------ | ------
0x6f1  | 0x18   | 0x02   | 0x21   | PID    | 0x00   | 0x00   | 0x00   | 0x00

### EGS response frame format

CAN ID | Byte 0 | Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 | Byte 7
------ | ------ | ------ | ------ | ------ | ------ | ------ | ------ | ------
0x618  | 0xf1   | 0x03   | 0x61   | PID    | 0x00   | 0x00   | 0x00   | 0x00

### EGS PIDs

PID  | response | description
---- | -------- | -----------
0x01 | TT       | EGS oil temperature (0x43 = ambient, perhaps °C - 50) XXX verify
0x0a | GG       | actual gear (GG = gear)
0x0b | LL       | EGS lockup (00 = not locked up)
0x0c | VV       | T30 voltage (0xae ~= 14v) XXX test
0x18 | SS       | selected gear (1 "P" 2 "R" 4 "N" 8 "D") XXX verify
0x1a | LL       | limp mode (89 = in limp mode)
