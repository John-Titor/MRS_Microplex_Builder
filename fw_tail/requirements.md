# E36 taillight module

## system

The vehicle electronic system consists of this module, the M57 DDE, ZF 6HP transmission controller (EGS), fuel pump controller (EKPM), MK60 ABS, AEM CD-7 dash, and another MicroPlex 7X operating as the PDM.

## pinout

| pin | signal  | function | description
|-----|---------|----------|-------------
| 1   | GND	    |		   |
| 2   | FUEL_IN | IN_1     | 0-5V fuel level signal
| 3   | ACC_MRS |          |
| 4   | CAN_L   |          |
| 5   | CAN_H   |          |
| 6   | T15_MRS | KL15     | Ignition-on signal
| 7   | -       | IN_2	   |
| 8   | -       | IN_3	   |
| 9   | REVERSE | OUT_4	   | reverse light/signal
| 10  | TAIL    | OUT_3	   | tail lights
| 11  | BRAKE_R | OUT_2	   | right brake light
| 12  | BRAKE_L | OUT_1	   | left brake light

## functionality

## brake lights
Operate the brake lights in accordance with the brake signal sent by the DDE.

### failsafe mode
If brake signal reporting from the DDE is lost, the brake lights shall flash 1s on / 1s off.

### attention-getter
If the brake is applied, and has not been applied for the last 4s, the brake lights shall alternate rapidly left and right. Each light will be illuminated for 250ms, for 3 cycles.

### night-time glare reduction
The brake lights shall operate at reduced brightness when the tail lights are on and the rain lights are not on.

### power-on reminder
The left and right brake light shall flash for 250ms every 4s, alternating one then the other, when power is on, the engine is not running, and the brake is not applied.

## fuel level
Fuel level will be monitored and reported every 1s as a percentage in the range 0-100%. 
The BMW fuel level message (0x349) expresses the level in volume units; it would require knowledge of the actual capacity of the cell. Could be more informative?

## tail lights
Operate the tail/rain lights in accordance with the lighting message sent by the PDM.

## reverse signal
Operate the reverse signal in 

## CAN messages of interest accordance with the lighting message sent by the PDM.

http://www.loopybunny.co.uk/CarPC/can/21A.html

| id       | details 
|----------|-----------------
| a8/168   | byte 8 = brake status; low values (<20) brake released, high values (>20) brake applied
| aa/170   | bytes 2/3 = throttle position 0-65535
|          | bytes 4/5 = engine speed * 4
| 21a/538  | byte 0 bit 2 = marker/tail lights on
|          | byte 0 bit 6 = rear fog/rain lights on
|          | byte 1 bit 1 = reverse light/signal on

https://github.com/nberlette/bmw-dbc/blob/main/src/bmw-e90.dbc

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
