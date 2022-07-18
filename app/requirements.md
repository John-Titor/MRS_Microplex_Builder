# E36 power distribution module

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

## future possibilities

 - move DDE / EGS scanning functionality from tail module
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
