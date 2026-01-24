DATASHEET – epc660
3D TOF imager 320 x 240 pixel
General Description
The epc660 is a fully integrated 3D-TOF imager with a resolution of
320 x 240 pixels (QVGA). As a system on chip, the epc660 con
tains next to the CCD pixel-field the complete control logic to oper
ate the device. The output of the chip is 12 bit DCS distance data
per pixel, which are accessible through a high-speed digital 12-bit
parallel video interface.
Only few additional components are needed to generate a com
plete 3D camera. Depending on illumination power and optical de
sign, a resolution in the millimeter range for distances up to dozens
of meters is feasible. Up to 158 full frame TOF images are deliv
ered in rolling mode. The extremely high sensitivity of the chip al
lows for a reduced illumination power and reduced overall power
consumption compared to other TOF imagers. 
epc660 is based on the same technology and instruction set as the
epc635 Half-QQVGA TOF imager from ESPROS.
An evaluation kit for the epc660 is available with hard- and soft
ware examples and a comprehensive manual to speed up system
integration.
Block Diagram
VDDA
VSSA
RESET
epc660
Applications
■ People detection and counting
■ Postal parcel size measurement
■ Machine safety
■ Drone near terrain flight assistance
■ ADAS systems
■ Pedestrian detection and breaking systems
■ Man-Machine interface
■ Gesture control
■ Body size measurement
■ General volumetric mapping
■ Mobile robotics
■ Simultaneous localization and mapping (SLAM)
ROW Buffer
VDDIO
VSSIO
VDD
VSS
RGU
SHUTTER
SCL
SDA
Charge 
Pump 
13V
EEPROM
I2C Slave
Pixel 
Cont.
&
Seq. 
Memory
Memory
Controller
&
Registers
ADC Read-Out
320x240
Pixel Field
4x Temp.
Sensors
at corners
De
modulator
ADC Read-Out
ROW Buffer
XTALOUT
XTALIN_CLKIN
m+n
Clock Delay
TCMI
OSC
PLL
0
1
DLL
CGU
VDDPLL
VSSPLL
MODCLK
Modulator
Figure 1: Functional block diagram
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
1 / 70
LED Driver
VDDPXH
VDDPXM
VSSPX
VBS
VSYNC_A0
HSYNC_A1
XSYNC_SAT
DATA[11:0]
DCLK
LED2 (push-pull)
LED (open-drain)
VSSLED
Datasheet_epc660-V2.20
www.espros.com
Main Features
Measurement Modes
■ General
□ 3D TOF imager in full monolithic design
□ 320 x 240 pixel-field, backside illuminated
□ QE >80% @ 850nm
□ Full well capacity 8'000 ke- (ambient and signal)
□ 39 fps full 3D TOF frame rate, single frame rate up to 158fps
□ Region of interest setting allows up to several kfps
□ 4 integrated temperature sensors
■ Measurement performance
□ Absolute accuracy in the sub-centimeter range with appropriate
setup and calibration
■ Integrated LED (or laser diode) driver
□ Laser diode (LD) illumination possible
□ Open-drain LED output pad, up to 200mA drive
□ Push-pull LED2 output pad, up to 50mA drive
■ Parallel digital data interface TCMI
□ 48MS/s max. data rate, 2.5/3.3V compatible
□ 12/8-bit parallel DATA output + XSYNC/SAT flag 
□ VSYNC, HSYNC and DCLK outputs
■ I2C control interface (slave)
□ 400kHz (FM) / 1MHz (FM+) 
■ Integrated EEPROM 128 x 8-bit
□ Calibration data and user programmable parameters
□ Unique chip ID
■ System / modulation clock
□ System clock 4MHz, internal by using crystal/resonator or us
ing external input
□ External LED/LD modulation input MODCLK (optional) up to 
96MHz
■ Power supply
□ Supply voltages +10V, +5V, +2.5/3.3V, +1.8V, -10V
□ Power consumption approx. 750mW (average)
■ Packaging
□ 9.7x8.7mm cost optimized 68pin CSP (chip scale package), 
□ Backside illuminated flip-chip SMD mounting
■ Other data
□ ROHS compatible
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
■ Illumination modulation modes
□ Sinusoidal modulation
□ Selectable modulation frequencies 0.75 ... 24MHz resulting in
unambiguity distance of 6.25m … 200m
■ Distance measurement modes
□  39 fps 3D TOF with 4x DCS frames, full pixel-field
□  79 fps 3D TOF with 2x DCS frames, full pixel-field
□ 158 fps 3D TOF with rolling read-out 4x DCS frames, full pixel
field
□ Ultra fast measurement by reduction of the image field (ROI)
□ SHUTTER release input for precise start/stop and single/con
tinuous measurement control
■ Non distance measurement modes
□ Ambient-light measurement (Grayscale image without illumina
tion)
□ Grayscale image with active illumination
Readout Modes
■ ROI (Region of interest)
□ Rectangular sub-pixel-field read-out
□ Increased frame rate
■ Binning and resolution reduction
□ Binning of max. 4 adjacent pixels (2 hor. and 2 ver.)
□ Resolution reduction to 2nd, 4th or 8th row or column to read-out
□ Increased frame rate for reduced number of rows
Figure 2: Picture of the epc660
2 / 70
Datasheet_epc660-V2.20
www.espros.com
Table of Contents
1.  Electrical, optical and timing characteristics ........................................................................................... 5
1.1.  Operating conditions and electrical characteristics ......................................................................................................................... 5
1.2.  Absolute maximum ratings .............................................................................................................................................................. 6
1.3.  Timing parameters .......................................................................................................................................................................... 6
1.4.  Optical characteristics ..................................................................................................................................................................... 6
1.5.  Sensitivity ........................................................................................................................................................................................ 7
1.6.  Ambient-light suppression (ABS) .................................................................................................................................................... 7
1.7.  Other optical parameters ................................................................................................................................................................ 8
1.8.  Temperature sensor characteristics ................................................................................................................................................ 9
1.9.  Distance measurement temperature drift ........................................................................................................................................ 9
2.  Pin-out ........................................................................................................................................................ 10
2.1.  Pin mapping .................................................................................................................................................................................. 10
2.2.  Pin list ........................................................................................................................................................................................... 10
2.3.  Power domain separation and ESD protection ............................................................................................................................. 13
3.  Packaging and layout information ........................................................................................................... 13
3.1.  Mechanical dimensions ................................................................................................................................................................. 13
3.2.  Parasitic light sensitivity (PLS) ...................................................................................................................................................... 14
3.3.  Pin1 marking ................................................................................................................................................................................. 15
3.4.  Location of the photosensitive area .............................................................................................................................................. 15
3.5.  PCB design and SMD manufacturing process considerations ...................................................................................................... 15
3.6.  Packaging information .................................................................................................................................................................. 16
4.  Ordering information ................................................................................................................................ 17
4.1.  Notes to various chip releases ...................................................................................................................................................... 17
5.  Hardware implementation ........................................................................................................................ 18
5.1.  Typical application diagram ........................................................................................................................................................... 18
5.2.  Application diagram part list .......................................................................................................................................................... 19
5.3.  Hardware implementation notes ................................................................................................................................................... 19
5.4.  Clock source ................................................................................................................................................................................. 21
5.5.  External modulation MODCLK ...................................................................................................................................................... 21
5.6.  Supply, reset and start-up options ................................................................................................................................................. 22
5.6.1.  Supply voltages and external reset ...................................................................................................................................... 22
5.6.2.  Start-up (Clock, PLL turn-on and EEPROM copy) ............................................................................................................... 23
5.6.3.  Strap pins ............................................................................................................................................................................ 23
5.7.  LED driver ..................................................................................................................................................................................... 24
5.8.  DLL (Delay Line) ........................................................................................................................................................................... 24
5.9.  Application system overview ......................................................................................................................................................... 25
6.  TOF camera interface (TCMI) ................................................................................................................... 26
6.1.  TCMI clock .................................................................................................................................................................................... 26
6.2.  Single or continuous measurement control ................................................................................................................................... 26
6.2.1.  Single measurement control ................................................................................................................................................ 26
6.2.2.  Continuous measurement control (auto-run) ....................................................................................................................... 26
6.3.  TCMI timing .................................................................................................................................................................................. 27
6.4.  TCMI data format .......................................................................................................................................................................... 28
6.5.  Frame rate and data-out performance .......................................................................................................................................... 30
6.5.1.  Frame rate QVGA 320x240 pixel (default) ........................................................................................................................... 30
6.5.2.  Frame rate Half QQVGA 160x60 pixel ................................................................................................................................. 31
6.5.3.  Memory space estimation QVGA ......................................................................................................................................... 31
7.  Pixel-field and architecture ...................................................................................................................... 32
7.1.  Pixel coordinates .......................................................................................................................................................................... 32
7.2.  Pixel saturation detection .............................................................................................................................................................. 33
7.2.1.  Hardware saturation flag ...................................................................................................................................................... 33
7.2.2.  Software saturation flag ....................................................................................................................................................... 33
8.  Operation modes ....................................................................................................................................... 34
8.1.  Full resolution mode (default) ........................................................................................................................................................ 34
8.2.  Dual phase mode (motion blur reduction) ..................................................................................................................................... 34
8.3.  Dual integration time mode (high dynamic range, HDR mode) ..................................................................................................... 35
8.4.  Pixel binning ................................................................................................................................................................................. 36
8.5.  Resolution reduction ..................................................................................................................................................................... 37
8.6.  Region of interest (ROI) ................................................................................................................................................................ 39
9.  Imaging ...................................................................................................................................................... 40
9.1.  Distance measurement (3D TOF) ................................................................................................................................................. 40
9.2.  Distance calculation algorithm ...................................................................................................................................................... 40
9.2.1.  Unambiguity range versus time base setting ....................................................................................................................... 41
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
3 / 70 Datasheet_epc660-V2.20
www.espros.com
9.2.2.  Quality of the measurement result ....................................................................................................................................... 43
9.3.  Grayscale imaging ........................................................................................................................................................................ 43
9.4.  Calibration and compensation of TOF cameras ............................................................................................................................ 43
9.5.  Noise reduction and signal filtering ............................................................................................................................................... 44
10.  Temperature sensors .............................................................................................................................. 45
10.1.  Initialization ................................................................................................................................................................................. 45
10.2.  Readout during runtime .............................................................................................................................................................. 45
10.3.  Calculate temperature in °C ........................................................................................................................................................ 46
11.  Application information .......................................................................................................................... 47
11.1.  Start-up and initialization sequence ............................................................................................................................................. 47
11.1.1.  Default ............................................................................................................................................................................... 47
11.1.2.  Customer specific .............................................................................................................................................................. 47
11.2.  Image acquisition ........................................................................................................................................................................ 47
11.2.1.  3D TOF mode .................................................................................................................................................................... 47
11.2.2.  Grayscale mode ................................................................................................................................................................. 47
11.2.3.  Dual phase mode selection (motion blur reduction) ........................................................................................................... 48
11.2.4.  Dual integration time mode selection (high dynamic range) ............................................................................................... 48
11.3.  Configuration sequence .............................................................................................................................................................. 49
11.4.  Integration time setting ................................................................................................................................................................ 49
11.5.  Power consumption ..................................................................................................................................................................... 50
11.6.  Rolling DCS frames ..................................................................................................................................................................... 51
11.7.  Enhanced rolling DCS frame mode ............................................................................................................................................. 51
12.  Parameter and configuration memory .................................................................................................. 53
12.1.  Data memory map ...................................................................................................................................................................... 53
12.1.1.  Control page ...................................................................................................................................................................... 53
12.1.2.  RAM page .......................................................................................................................................................................... 53
12.1.3.  EEPROM page .................................................................................................................................................................. 53
13.  I2C interface ............................................................................................................................................. 54
13.1.  Device addressing ...................................................................................................................................................................... 54
13.2.  I2C bus protocol notation ............................................................................................................................................................. 54
13.3.  I2C bus timing .............................................................................................................................................................................. 54
13.4.  I2C commands ............................................................................................................................................................................. 55
13.4.1.  Software reset ................................................................................................................................................................... 55
13.4.2.  Device address reload ....................................................................................................................................................... 55
13.4.3.  Write single-byte ................................................................................................................................................................ 55
13.4.4.  Write multi-byte .................................................................................................................................................................. 55
13.4.5.  Read single-byte ................................................................................................................................................................ 56
13.4.6.  Read multi-byte ................................................................................................................................................................. 56
13.5.  Command timing ......................................................................................................................................................................... 57
14.  Register map ........................................................................................................................................... 57
14.1.  Control page 0x00 ~ 0x7F ........................................................................................................................................................... 58
14.2.  RAM page (0x80 ~ 0xEF) ........................................................................................................................................................... 59
14.3.  EEPROM page, indirect data access section (0xF0 ~ 0xFF) ...................................................................................................... 63
15.  Control command examples .................................................................................................................. 64
15.1.  I2C control command examples: ................................................................................................................................................. 64
15.2.  Software reset with I2C general call command ............................................................................................................................ 64
15.3.  4 DCS: Acquire DCS0 … 3 frames with tint = 16.6µs @ 12MHz modulation frequency ................................................................ 64
15.4.  4 DCS: Acquire DCS0 … 3 frames with tint = 16.6µs, followed by DCS 0 … 3 with tint 333µs @ 12MHz mod. frequency ......... 64
15.5.  2 DCS: Acquire DCS0 and 1 with tint = 16.6µs @ 12MHz modulation frequency ......................................................................... 64
15.6.  Indirect single write to EEPROM: Store 1 byte at user register 0xF0 .......................................................................................... 64
15.7.  Indirect single read from EEPROM: Read 1 byte from user register 0xF0 .................................................................................. 64
15.8.  Reading part version (register 0xFB) .......................................................................................................................................... 65
15.9.  Reading IC version (register 0x01) ............................................................................................................................................. 65
15.10.  Reading WAFER ID and CHIP ID ............................................................................................................................................. 65
15.11.  Pixel sequencer code write procedure ...................................................................................................................................... 65
15.12.  Pixel sequencer code ................................................................................................................................................................ 65
15.13.  Pixel sequencer code read back ............................................................................................................................................... 68
16.  Addendum ............................................................................................................................................... 69
16.1.  Terms, definitions and abbreviations ........................................................................................................................................... 69
16.2.  Related documents ..................................................................................................................................................................... 69
17.  IMPORTANT NOTICE ............................................................................................................................... 70
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
4 / 70 Datasheet_epc660-V2.20
www.espros.com
1. Electrical, optical and timing characteristics
All characteristics are at typical operational ratings, TA = +25ºC, modulation frequency 12MHz, supply voltages at nominal value, unless
otherwise stated
1.1. Operating conditions and electrical characteristics
Parameter Description Conditions/Comments Min. Typ. Max. Units
VDD,VDDPLL Digital supply voltage Ripple 1 < ± 20 mV 1.71 1.80 1.98 V
VDDIO IO supply voltage 3 Ripple 1 < ± 50 mV 2.25 2.5/3.3 3.63 V
VDDA, VDDPXM Analog 1 supply voltage 2 Ripple 1 < ± 20 mV 4.9 5.0 5.1 V
VDDPXH Analog 2 supply voltage 2 Ripple 1 < ± 20 mV 9.5 10 10.5 V
VBS Bias supply voltage Ripple 1  < ±50 mV-10.5-10.0-9.75 V
IVDD Total digital supply current, including PLL
supply current
14 20 mA
IVDDPLL PLL supply current 4 mA
IVDDIO IO supply current 4 8 mA
IVDDA Analog supply current refer to chapter 11.5 125 350 mA
IVDDPXM Analog 1 supply current 1 mA
IVDDPXH Analog 2 supply current 13 mA
IVBS Bias supply current 8 3.8 8 mA
VLED_ON LED on-voltage forward voltage @ ILEDOD-ON = 100 mA
@ ILEDOD-ON = 200 mA
0.1
0.2
V
V
ILED_LEAK LED leakage current @ LEDOD off-voltage 10 µA
ILED2_SINK LED2 output sink/source current 50 mA
VIH_VDDIO Digital high level input voltage 5 excluding XTALIN 0.7 x VDDIO V
VIL_VDDIO Digital low level input voltage 5 excluding XTALIN 0.3 x VDDIO V
VIH_XTALIN Digital high level input voltage XTALIN 1.35 V
VIL_XTALIN Digital low level input voltage XTALIN 0.2 V
VOH Digital high level output voltage 5, 6 0.8 x VDDIO V
VOL Digital low level output voltage 5, 6 0.2 x VDDIO V
RPD Pull-down resistor in RESET, 
VSYNC_A0, HSYNC_A1
600 kΩ
IIH Digital high level input current 7 VIH max. 10 7 µA
IIL Digital low level input current 7 VIL min.-10 7 µA
IOH Digital output source current 7 VOH max. 50 mA
IOL Digital output sink current 3 VOL min.-50 mA
CIO IO load capacitance 5 30 pF
fIO IO switching frequency 5 24 48 MHz
PPk Power dissipation (average) See Table 30 750 mW
RTh Thermal resistance on PCB with underfill 40 °K/W
TOP Operating temperature-40 105 °C
Table 1: Operating conditions and electrical characteristics
Notes:
1 Min. and Max. voltage values include noise and ripple voltages.
2 Analog voltage supplies have direct influence on measurement performance. They must be properly decoupled for low noise and ripple.
3 IO voltage supply must be equal to external processor's IO supply voltage levels used in the application. It can be set to any value within
min and max. operating voltage.
4 When device is operated at max fDCS frame rate, DCLK at 48MHz, driving loads 15pF each. 
5 I2C pins SCL and SDA are open-drain outputs and need termination (pull-up resistor) according to I2C standards.
6 VOH/OL and IOH/OL values are measured at max CIO and max fIO.
7 Value is without termination resistors
8 A bright illuminated white target right in front of the chip with lens leads to an IVBS of approx. 3.8 mA, without any illumination approx. 
3.6 mA and with strong illumination (approx. 55 mW/cm2, no lens) typ. 17 mA.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
5 / 70 Datasheet_epc660-V2.20
www.espros.com
1.2. Absolute maximum ratings
Parameter Conditions
Supply voltage VDD, VDDPLL-0.5V ... +2.0V
Supply voltage VDDIO, VDDA, VDDPXM-0.5V ... +5.5V
Supply voltage VDDPXH-0.5V ... +13.5V
Supply voltage VBS-12.0 ... +0.5V
Voltage to any pin in the same VSC supply class. VSC min - 0.3V … VSC max + 0.3V
LED sink current ION_LED (modulated peak current, refer to Figure 18) 200 mA @TJ
 85°C
25 mA @TJ
 125°C
linear reduction between 85 and 125°C
LED off-voltage VOFF_LED (open-drain output) 7.5 V
ESD rating JEDEC HBM class 1C (1kV to < 2kV)
Junction temperature (TJ
)-40°C to +125°C
Relative humidity 0 ... 95%, non-condensing
Table 2: Absolute maximum ratings
1.3. Timing parameters
Parameter Description Conditions Min. Typ. Max. Units
tSTARTUP Start-up time after applying external supplies 340 1'000 µs
tRESET RESET 100 ns
tPLLStrap_scan Scanning strap pins 4x osc_clk
tPLL PLL lock time 30 µs
tDLL DLL delay for 1 step approx. 30cm distance shift per step. Refer for de
tails to register 0x73 and Figure 23, for exact 
value to register 0xE9.
2.1 ns
tDRV Illumination driver delay delay of LED/LED2 versus demodulation,
refer to Figure 59
8.4 ns
tEEPROM_to_CFG Load CFG registers copy EEPROM to CFG registers 340 µs
tEEPROM_Write Write EEPROM waiting time per byte 25 ms
fXTAL Clock frequency determines the distance measurement accuracy 3.8 4 4.2 MHz
dfXTAL Clock frequency deviation any deviation is added as a linear distance error ±100 ppm
fJITTER Clock frequency phase jitter peak-to-peak, cycle to cycle 50 ps
fLED LED modulation frequency 0.75 24 MHz
fMODCLK Ext. modulation clock refer to chapter 5.5 96 MHz
tLED_rise/fall Rise/fall time LED/LD 12 ns
fDCLK TCMI pixel rate 12 bit pixel data + saturation flag 24 48 MHz
fTCMI_data TCMI data rate 312 624 Mbit/s
fSCL I2C data rate 1 Mbit/s
Table 3: Timing parameters
1.4. Optical characteristics
Parameter Description Conditions/Comments Value Units
APIXEL Pixel photosensitive area 100% fill factor 20 x 20 μm
ASENSOR Pixel-field area 320 x 240 pixel 6.4 x 4.8 mm
Table 4: Optical characteristics
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
6 / 70 Datasheet_epc660-V2.20
www.espros.com
1.5. Sensitivity
@ integration time 100 µs
Parameter Description Min. Typ. Max. Units
TOF sensitivity STOF 
• Modulation frequency 12MHz
• Amplitude 1,400 LSB
640nm 0.75 0.9 1.05
850nm 0.50 0.6 0.70
940nm 0.65 0.8 0.95
TOFSENS
 FPN Sensitivity fix pattern noise, @ 1,400 LSB 40 100 LSB
TOFDIST
 FPN Distance fix pattern noise, @ 1,400 LSB 18 50 mm
IDark
Dark current (drift during readout) 10 20 LSB/ms
Grayscale sensitivity
Normal operation 0.19 0.25 0.31
Temperature sensing mode 0.48 0.62 0.76
Hν Optical sensitivity 150k LSB
Lux/sec
GSSTD
Grayscale standard deviation 25 100 LSB
Table 5: Sensitivity
1.6. Ambient-light suppression (ABS)
An important function of the 3D TOF pixel is the ambient-light suppression. It removes DC or low frequency modulated light caused by 
sunlight, room illumination, etc. from the modulated light generated by the camera illumination. The amount of collected ambient light is 
proportional to the integration time. The longer the integration time, the more unwanted light will be collected. It's a good practice to keep 
the integration time for TOF imaging below 1ms. In addition, optical bandpass filters to block the unwanted light spectrum is mandatory.
Parameter Ambient light suppression Integration
time
Wave
length
Min. Typ. Max. Units
Ee Irradiance, DC light 100 µs 640nm 0.30 mW/mm2
850nm 0.20
940nm 0.25
Parameter Ambient light suppression Integration
time
Center
wavelength
Bandwidth Min. Typ. Max. Units
Ev
Luminance equivalent, sunlight 500µs 640nm ±27.5nm 85 kLux
850nm ±32.5nm 70
940nm ±30nm 190
Table 6: Ambient light suppression
Note:
The default and suggested chip configuration is set to achieve highest possible frame rate and using additional ambient-light correction ac
cording the Application note AN10 Calibration and compensation: Register 0x90, bit 3 = 0 and 0xAB = 0x00. A 20% more efficient ambient
light suppression is possible, if the the following registers are modified:
0x90, bit 3 = 1
0xAB = 0x01
It turns the LED modulation before each integration for additional 33µs @ 24MHz modulation frequency on. This modulation is indepen
dent of the effective integration time. The on-time depends on the modulation frequency by tON = 40µs * 20MHz / modulation frequency.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
7 / 70 Datasheet_epc660-V2.20
www.espros.com
nW/mm2
LSB
nW/mm2
LSB
1.7. Other optical parameters
400 500 600 700 800 900 1000
0
10
20
30
40
50
60
70
80
90
100
Wavelength [nm]
Relative sensitivity [%]
0 10 20 30 40 50 60 70 80 90100
0
1
10
100
Angle of incidence [deg]
Reflectance [%]
630nm
500nm
850nm
950nm
Figure 3: Relative spectral sensitivity (Sλ) vs. wavelength Figure 4: Reflectance vs. illumination angle (AOI)
30
100
Mean amplitude ATOF [LSB]
1-σ temporal distance noise [mm]
500 1000 1500 2000 0
20 10 40
operating range (useful amplitude)
300 400 500 600 700 800 900 1000 1100
0
10
20
30
40
50
60
70
80
90
100
Wavelength [nm]
QE [%]
Figure 5: Typ. distance noise, single shot, 
4 DCS, no ambient-light, see chapter 9.2.2
Figure 6: Typical quantum efficiency-40-20 0 20 40 60 80 100
0
0.2
0.4
0.6
0.8
1
1.2
Temperature [°C]
Normalized sensitivity
Figure 7: Typical TOF sensitivity temperature coefficient
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
8 / 70 Datasheet_epc660-V2.20
www.espros.com
1.8. Temperature sensor characteristics
Parameter Description
Conditions
Min.
Typ.
Max.
Units
TTEMP
Measurement range-40
+105
ºC
PTEMP
Sensor resolution
14
bit
k
Temperature sensor gain
0.067
K/LSB
Lin
Linearity
Over temperature range
5
%
TCAL
Calibration temperature
Note: Refer also to chapter 10.
1.9. Distance measurement temperature drift
@12MHz modulation frequency
26.5
Table 7: Temperature sensor characteristics
27.0
27.5
°C
Parameter Description
Min.
Typ.
Max.
Units
TCPIX
Pixel
11.3
mm/K
TCOD
LED/LD driver
2.7
mm/K
TCDLLn
DLL stage, per stage
0.65
Table 8: Optical characteristics
mm/K
Note: Values vary from imager to imager. Refer for details to Figure 22 and application note AN10 Calibration and Compensation, chapter 
temperature compensation.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
9 / 70
Datasheet_epc660-V2.20
www.espros.com
2. Pin-out
2.1. Pin mapping
XSYNC_SAT
DATA0
DATA1
VSSIO1
VDDIO1
DATA2
DATA3
VSS1
VDD1
DATA4
DATA5
VSSIO2
VDDIO2
DATA6
DATA7
DATA8
VDDIO3
VSSIO3
DATA10
PIN22
PIN23
PIN24
PIN25
SCL
SDA
VDD2
VSS2
PIN31
DATA11
VSSLED
PIN49
VDDPXM2
VDDPXH2
VSSPX2
VSSA2
VDDA2
PIN43
VDDA1
VSSA1
VSSPX1
VDDPXH1
VDDPXM1
PIN52
PIN53
RESET
SHUTTER
VSS3
VDD3
XTALIN_CLKIN
XTALOUT
VSSPLL
VDDPLL
VSYNC_A0
HSYNC_A1
DCLK
VDDIO4
VSSIO4
MODCLK
PIN32
PIN33
PIN34
VBS
PIN30
LED2
PIN36
PIN35
LED
DATA9
epc660
9.68 x 8.64mm CSP housing
200µm solder ball diameter
19 x 17 balls, 500um ball pitch
O x
y
 
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
49
48
47
46
45
44
43
42
41
40
39
38
37
36
35
51 52 53 54 55 56 57 58 59 60 61 62 63 64
32 31 30 29 28 27 26 25 24 23 22 21 20 19 18 33
65 66
34
50
67 68
Figure 8: CSP pin mapping (top-view, solder balls are at the bottom, pixel-field is at the top)
2.2. Pin list
Pin
No.
Pin name Supply
class 
VSC
Pin 
type
RESET 
function
RESET 
level
Description
IO pins
2 DATA0 VDDIO DIO IPD VOL TCMI high-speed output bit 0, no pull-up resistor allowed.
3 DATA1 VDDIO DIO IPD VOL TCMI high-speed output bit 1
6 DATA2 VDDIO DIO IPD VOL TCMI high-speed output bit 2
7 DATA3 VDDIO DIO IPD VOL TCMI high-speed output bit 3
10 DATA4 VDDIO DIO IPD VOL TCMI high-speed output bit 4
11 DATA5 VDDIO DIO IPD VOL TCMI high-speed output bit 5
14 DATA6 VDDIO DIO IPD VOL TCMI high-speed output bit 6
15 DATA7 VDDIO DIO IPD VOL TCMI high-speed output bit 7
Table 9: Pin list
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
10 / 70 Datasheet_epc660-V2.20
www.espros.com
Pin
No.
Pin name Supply
class 
VSC
Pin 
type
RESET 
function
RESET 
level
Description
16 DATA8 VDDIO DIO IPD VOL TCMI high-speed output bit 8
17 DATA9 VDDIO DIO IPD VOL TCMI high-speed output bit 9
20 DATA10 VDDIO DIO IPD VOL TCMI high-speed output bit 10
21 DATA11 VDDIO DIO IPD VOL TCMI high-speed output bit 11
65 DCLK VDDIO DIO IPD VOL TCMI data clock output
63 VSYNC_A0 VDDIO DIO IPD VOH TCMI VSYNC output / strap input 0, refer to chapter 5.6.3
64 HSYNC_A1 VDDIO DIO IPD VOH TCMI HSYNC output / strap input 1, refer to chapter 5.6.3
1 XSYNC_SAT VDDIO DIO IPD VOH TCMI XSYNC / TCMI saturation flag output, no pull-up resistor allowed.
26 SCL VDDIO DIOD I VIH I2C clock input 4
27 SDA VDDIO DIOD I VIH I2C data input/output 4
56 SHUTTER VDDIO DI PD VIL Shutter input 5
55 RESET VDDIO DI PD VIL Reset input (active low), 600kΩ int. pull-down 3
68 MODCLK VDDIO DI PD Modulator/demodulator external clock input.
54 LED2 VDDIO DO LED driver push-pull output 2
22 PIN22---DO VOL
Do not make any electrical connection except to a test pad. 23 PIN23---DI PU VIH
24 PIN24---DI PD VIL
25 PIN25---DI PU VIH
Digital pins
59 XTALIN_CLKIN VDDPLL AI XTAL or Resonator in / CLKIN from external clock source
60 XTALOUT VDDPLL AO XTAL or Resonator out
Analog pins
51 LED VDDLED AOD VLED max LED/LD driver open-drain output 2
35 PIN35 VDDPXH--
Connect to VSSPX with 10 kOhm
36 PIN36 VDDPXH AI
31 PIN31---AI
Do not make any electrical connection except to a test pad. 32 PIN32---AI
33 PIN33-----
34 PIN34-----
49 PIN49---AI Connect to ground with a 10kΩ resistor 
52 PIN52------Do not make any electrical connection except to a test pad.
53 PIN53-----
Supply pins, digital
5 VDDIO1 VDDIO PWR
IO supply VDDIO
13 VDDIO2 VDDIO PWR
18 VDDIO3 VDDIO PWR
66 VDDIO4 VDDIO PWR
9 VDD1 VDD PWR
Digital supply VDD 28 VDD2 VDD PWR
58 VDD3 VDD PWR
62 VDDPLL VDDPLL PWR PLL supply
4 VSSIO1 VDDIO GND
IO ground VSSIO
12 VSSIO2 VDDIO GND
19 VSSIO3 VDDIO GND
67 VSSIO4 VDDIO GND
Table 9 cont.: Pin list
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
11 / 70 Datasheet_epc660-V2.20
www.espros.com
Pin
No.
Pin name Supply
class 
VSC
Pin 
type
RESET 
function
RESET 
level
Description
8 VSS1 VDD GND
Digital ground VSS 29 VSS2 VDD GND
57 VSS3 VDD GND
61 VSSPLL VDDPLL GND PLL ground
Supply pins, analog
42 VDDA1 VDDA PWR
Analog supply VDDA
44 VDDA2 VDDA PWR
37 VBS VBS PWR Bias supply
39 VDDPXH1 VDDPXH PWR
Pixel analog 2 supply VDDPXH
47 VDDPXH2 VDDPXH PWR
38 VDDPXM1 VDDPXM PWR
Pixel analog 1 supply VDDPXM
48 VDDPXM2 VDDPXM PWR
41 VSSA1 VDDA GND
Analog ground VSSA
45 VSSA2 VDDA GND
40 VSSPX1 VDDPX GND
Pixel analog ground VSSPX
46 VSSPX2 VDDPX GND
50 VSSLED VDDLED GND LED/LD driver ground (return current) 1
30 PIN30 VPIN35 PWR Connect to VSS
43 PIN43 VPIN43 PWR Connect to VDDA
Table 9 cont.: Pin list
Notes:
1 VSSLED is the dedicated, isolated GND pin for the LED/LD return-current from external circuitry. It must be connected to PCB GND
plane together with the other VSSA GND pins.
2 LED output can be used to drive an external amplifier with an addition of a pull-up resistor. The voltage at LED output pin must not ex
ceed value in Table 1: Operating conditions and electrical characteristics.
LED2 output is a push-pull driver for delivering symmetric rise/fall times to the external LED driver circuit. LED2 is internally connected to
VDDIO/VSSIO supplies. During integration time, all TCMI pins are silent except for DCLK. As a result, LED2 pin will not pick up switch
ing noise from all other TCMI pins but the layout has to take care of the DCLK line. 
LED and LED2 must not be used simultaneously for driving LED circuits on the PCB. They exhibit different insertion delays and may
cause unpredicted distance offset/measurement results.
3 RESET pin has a 600kΩ (typical) internal pull-down resistor. Therefore, this pin can be safely connected to a standard GPIO of a CPU
which is initially high-Z or open-drain during power up sequence. Once the SW takes control, it can program this GPIO as output and
drive 1 to release the RESET. The internal pull-down can be override by and external 10kΩ pull-up and a series capacitor to build a sim
ple delayed power-on reset for evaluation/qualification purposes.
4 I2C pins SCL, SDA are according to I2C standards. They are I2C slave pins which need external pull-up resistors on the PCB. Values of
R1 and R2 in the schematics are given only for indicative purposes and must be re-calculated according to the total capacitive load of all
I2C slave/master devices and operating mode (FM or FM+) of the I2C (chapter 13) in the application.
5 If HW shutter is not used, connect this pin to GND
'Pin type' in Table 9 defines the following:
■DI: Digital Input
■DO: Digital Output
■DIO: Digital Input/Output (bidirectional)
■DIOD: Digital Input/Output (bidirectional), open-Drain
■AI: Analog Input
■AO: Analog Output
■AOD: Analog Output, open-Drain
■PWR: Supply
■GND: Ground
'Rst. Func.' in Table 9 defines the function of IO pins during reset:
■I: Input
■PU: internal Pull-Up
■PD: internal Pull-Down
■IPD: Input with internal Pull-Down
'Rst. Level' in Table 9 defines the level of the IO pins during/after
reset (chapter 5.6)
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
12 / 70 Datasheet_epc660-V2.20
www.espros.com
2.3. Power domain separation and ESD protection
The epc660 chip has internally 10 different power domains and 6 ground references which are interconnected with ESD protection diodes.
All pins are also equipped with ESD protection diodes. The diodes have a breakthrough voltage of 0.3V. The designer has to take care that
none of these diodes become conductive either at power-up, power-down or normal operation.
epc660
XSYNC_SAT
DATA[11:0]
SHUTTER
VSYNC_A0
HSYNC_A1
DCLK
MODCLK
SCL
SDA
LED2
PIN22
PIN23
PIN24
PIN25
LED
PIN31
PIN32
PIN33
PIN34
PIN35
PIN36
VSSLED
VSSLED
VSSIO
VSSIO VDDIO
VSSPX
VSSPX
VDDPXH
VDDPXM
VDDA
VBS
PIN52
PIN53
VSSPLL
VSSPLL
VDD
VDDPLL
XTALIN_CLKIN
XTALOUT
VSS
VSSA
VSSA
VSS
PIN30
PIN43
RESET
PIN49
Figure 9: I/O pins and ESD protection diagram
3. Packaging and layout information
3.1. Mechanical dimensions 
1
2
1
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17 18
10
19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35
36
37
38
39
40
41
42
43
44
45
46
47
48
49
50
51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68
(0.330) 
0.500
4.900 
9.70 +0.00/-0.08 
9.000 (19 bumps with 0.5mm pitch) 
6.400 
4.800 
(0.330) 
0.500
4.000 
8.000 (17 bumps with 0.5mm pitch) 
8.66 +0.00/-0.08 
C4
R6
R245
C323
Photosensitive area:
on top side,
320 x 240 pixel
Pin 1
Top side
Solder bumps
Top side view
0.19 ±0.02 
(0.050)
(∅ 0.200)
Pixel size:
20μm x 20μm
column
row
Figure 10: Mechanical dimensions
Notes:
■
■all measures in mm
■not specified tolerances: ±0.001mm
■Dimensions in brackets informal only
■Top side is illumination side
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
13 / 70 Datasheet_epc660-V2.20
www.espros.com
3.2. Parasitic light sensitivity (PLS)
CMOS circuits are sensitive to light. That is why they can be used for photo-sensing, imaging, etc. However, if strong light is radiating the 
chip beside the pixel field, analog and digital circuits can be affected in its function by such parasitic light. It is called parasitic light sensitiv
ity (PLS). A known effect is a shift of the measured distance under strong ambient light. Imager lenses have always a larger field of view 
than the pixel-field area. In order to prevent the chip being illuminated by strong ambient light, an opaque aperture should be placed onto 
the photosensitive side of the imager as shown in Figure 11. The cover shall have a opening of 6.690 x 5.090 mm. With regard to the 
6.400 x 4.800 mm pixel-field size, this shield can be assembled with a tolerance of ±120 μm in x and y axis. Such a cover can be made by 
a thin sheet metal stencil like an SMD solder paste printing stencil or by silk screen printing of black color.
Top side view
Opening: 6.690 ±0.050
6.400 
Pin 1
1
1
2
3
68
67
66
65
64
63
62
61
60
59
58
57
C4
4
R6
5
6
Opening: 5.090 ±0.050
7
column
row
8
4.800 
9
10
10
11
12
13
14
15
16
Photosensitive area:
on top side
R245
Minimize gap to reduce
stray-light underneath
the cover
(0.330) 
56
55
54
53
C323
52
51
50
49
48
47
46
4.000 
45
44
43
42
41
40
39
17 18
38
37
36
19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35
(0.330) 
4.900 
Chip size: 9.70 +0.00/-0.08 
Chip size: 8.70 +0.00/-0.08 
Light absorbing cover
with opening at pixel-field,
nor reflective nor transparent,
blocking ambient-light 
surrounding the pixel-field
Figure 11: Opaque cover for protection against unwanted ambient-light
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
14 / 70
Top side
Datasheet_epc660-V2.20
www.espros.com
3.3. Pin1 marking
The following pictures shows the epc660 chip from the bottom side with view to the solder balls. Please note the location of pin 1. It's
highly recommended to check the pin 1 orientation with a vision system during the SMT assembly process.
Pin 1
Pin 1
epc660 chip from the solder ball side
3.4. Location of the photosensitive area
Top right corner from the solder ball side
Figure 12: Pin 1 marking 
The photosensitive area is not marked neither on the front nor on the backside of the IC. As a visible reference, a metal ring of the IC can
be used. From the solder ball side it is visible. Also from the front side (photosensitive area) it can be seen with a camera which is sensitive
in the near infrared wavelength domain (950 .. 1'150nm).
3.5. PCB design and SMD manufacturing process considerations
As the epc660 chip comes in a 68 pin chip scale package with only 50μm thickness, the PCB layout should be made with special care. In
addition, careful handling during the assembly process shall be assured in order to avoid mechanical damage during the assembly
process. Because the silicon chip is small and light weight compared the solder balls, it is highly recommended that all tracks to the chip
should come straight from the side. A symmetrical design is highly recommended to achieve high production yield. The pads and the tracks
should also have exactly the same width at least for 1mm from the pad. They shall be covered by a solder resist mask in order to avoid
drain of the solder tin alloy to the track.
As shown in Figure 13, a ground plane shall be placed on the top PCB layer underneath the chip. This ground plane is the common GND
point and acts as a shield to suppress high frequency emission of fast interface signal lines. It is important that this plane is completely flat.
Thus, the plane must not be scattered nor divided into sections. It should be rather full-faced and evenly plane for vias placed underneath
this plane. Otherwise chip bending might occur. In addition, the ground plane helps to dissipate the heat generated by the chip operation. A
good heat dissipation is achieved if there is a temperature increase of the chip under normal operation of max. 20K. The temperature can
be read direct from the chip.
Underfill of the component reduces stress to the solder pads caused by e.g. temperature cycling or mechanical bending. Furthermore the
thermal and mechanical fatigue will be reduced and the longterm reliability will be increased. Underfill material and underfill selection is ap
plication specific. It shall follow JEDEC-STD JEP150: Stress-Test-Driven Qualification of and Failure Mechanisms Associated with Assem
bled Solid State Surface- Mount Components. Please also, refer to the application note AN08 Process-Rules CSP Assembly which can be
downloaded from the ESPROS Website at www.espros.com, section Downloads. Obeying these recommendations is very important to
achieve a high manufacturing yield and high reliability. 
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
15 / 70
Datasheet_epc660-V2.20
www.espros.com
Track:
0.150
Pad:
⌀0.230
Solder stop opening:
⌀0.350
Pitch: 0.500
Pitch: 0.500
Solder stop
covers PCB
and tracks
Common ground plane- Connects all analog and digital GNDs together- Fully covered with solder stop- Thermal heat-sinking (vias) to the PCB backside- Vias must be filled solid or covered by solder mask
  to prevent drain of underfill into or through the vias
Note
The chip's optical performance is not
affected if the unevenness of the PCB
board surface structure is visible on top
of the chip's surface.
Pin1
1mm underfill frame around the chip. 
Don't place components within this frame.
Figure 13: Recommended PCB layout (all measures in mm)
3.6. Packaging information
The devices will be shipped in standard JEDEC trays for automatic placement systems. General tray specification data are available in a
separate datasheet. Further tray specifications can be found in the JEDEC Association standard JEP95.
The chips are placed according industry standard with pin 1 at the tray chamfer corner, refer to Figure 14. ESPROS does not guarantee
that there are no empty cavities. Thus, the pick-and-place machine should check the presence of a chip during picking. In addition, it
should verify the correct location of pin 1 (refer to Figure 12).
The trays are designed for vacuum pick-up and for a maximum temperature of 150ºC.
Trays are packed and shipped in multiples of single trays with an empty cover tray on top. Trays are not a hermetic packaging. Thereof for
storage and transportation, the tray stack is sealed in a moisture barrier bag.
11.80
10.00
16.00
112.00 +/-0.25
295.00 +/-0.25
135.9
315.0
322.6
11.95
Chamfer
Pin 1
Figure 14: JEDEC tray for 26 x 8 pieces, maximum quantity 208 pieces per tray, use vacuum pick-up
(all measures in mm)
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
16 / 70 Datasheet_epc660-V2.20
www.espros.com
4. Ordering information
Figure 15: epc660-CSP68 bottom and top side
Figure 16: epc660 CC Chip Carrier,
refer to separate datasheet
Part Number
Part Name
Package
RoHS compliance
P100 183
epc660-CSP68
CSP68
Yes
P100 244
epc660 CC Chip Carrier
PCB 37.25 x 36.00 mm
Table 10: Ordering Information
4.1. Notes to various chip releases
The supplied chip version can be identified by 
Yes
■ reading the extension -XXX of the part name on the packaging labels or delivery papers: epc660-CSP68-XXX.
■ reading the part version register 0xFB: Refer to chapter 15.8.
■ The latest download code for each chip version is included in the download package for the epc660 Evaluation Kit (see chapter 15.11).
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
17 / 70
Datasheet_epc660-V2.20
www.espros.com
5. Hardware implementation
5.1. Typical application diagram
16
15
14
13
12
11
10
9
8
7
6
5
4
3
1
2
XSYNC_SAT
VSSIO2
DATA5
DATA4
VDD1
VSS1
DATA3
DATA2
VDDIO1
VSSIO1
DATA1
DATA0
DATA8
DATA7
DATA6
VDDIO2
33
31
30
29
28
27
26
25
24
23
22
21
20
19
17
18
DATA9
VDD2
SDA
SCL
PIN25
PIN24
PIN23
PIN22
DATA11
DATA10
VSSIO3
VDDIO3
PIN33
PIN31
PIN30
VSS2
51
50
49
48
47
46
45
44
43
42
41
40
39
38
36
37
PIN36
VSSPX2
VSSA2
VDDA2
PIN43
VDDA1
VSSA1
VSSPX1
VDDPXH1
VDDPXM1
VBS
VSSLED
LED
PIN49
VDDPXM2
VDDPXH2
68
67
66
65
64
63
62
61
60
59
58
57
56
55
53 PIN53
HSYNC_A1
VSYNC_A0
VDDPLL
VSSPLL
XTALOUT
XTALIN_CLKIN
VDD3
VSS3
SHUTTER
RESET
MODCLK
VSSIO4
VDDIO4
DCLK
34 PIN34
35 PIN35
52 PIN52
32 PIN32
epc660
C28
1μ
C27
10n
C26
1μ
C25
10n
C24
1μ
C23
10n
C34
1μ
C33
10n
C32
1μ
C31
10n
C30
1μ
C29
10n
C36
1μ
C35
10n
C4
100n
C8
100n
C7
10μ
C2
100n
C6
100n
C1
10μ
C5
10μ
C16
100n
C15
1μ
C10
100n
C9
1μ
C14
100n
C13
10μ
C12
100n
C11
10μ
L2
100n
C22
100n
C21
1μ L3
100n
VBS
(-10V)
R2
15
LED1
LEDn
R1
1k2
R3
0 or
10k
C18
100n
C17
100μ
VSSLED
VDDLED
Low Z, fast switching connection
(as short and wide conductors as possible)
X1
4.0 MHz
C20*
18p
C19*
18p
NC
NC
NC
GND
VDDA
(+5V)
L1
100n
VDDPXH
(+10V)
VDDIO
(2.5/3.3V)
VSS
VDDD
(1.8V)
R4 - R19
0 or 10 – 33
Connection to
the common
GND plane
VSSIO
Supply class
VSS
VSSPX
VSSA
VSSLED
* value according manufacturer data
VSSPLL
+2.5/+3.3V digital_1
GND domains Voltage
VDDIO
VDD
VDDPLL
VDDA
VDDPXH
VDDLED
+1.8V digital_2
+1.8V digital_3
+5.0V analog_1
+10.0V analog_4
VDDPXM +5.0V analog_5 VSSPX
VSSA VBS-10.0V analog_3
Note 1
Note 1: VDDLED is application specific (LED drive voltage)
maximum
 +7.0V
R20
1k
R21
1k
I2C termination
VDD_CPU
VSS_CPU
C37
100n
C38
10μ
STRAP
pins
Application
CPU
R22
open or 10k
R23
open or 10k
NC
NC
NC
NC
NC
VSS
NC
NC
NC
Note 2: It is suggested, to have not connected pins (NC)
 on test pads available.  
R25
0 or 10k
R24
0 or 10k
54 LED2
Note 3: Common GND plane underneath the chip suggested. 
             Refer to recommended PCB layout
IMPORTANT: SHUTTER and XSYNC_SAT must
be at low level during power up !
C3
10μ
Figure 17: Typical application diagram
Notes:
R4-R19 Resistor value depends on fast bus decoupling, typically 33 Ohms.
R3, R24, R25 Pins need to be connected to GND. In case of need to testability: use 10k resistors.
R22, R23 Resistor value depends of needed strap function.
SHUTTER and XSYNC_SAT Must be at low level until RESET release. Otherwise malfunction occurs.
VSS Connect all VSS as direct as possible using vias to the GND plane underneath the imager 
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
18 / 70 Datasheet_epc660-V2.20
www.espros.com
5.2. Application diagram part list
Part
designator
Description Pin No. Value Toler
ance
Supply 
class VSC
Comments
Min. Typ. Max.
Minimum part count:
C11 VDDA 41 – 42, (44 – 45) 10 μF ±20% VDDA Ceramic X7R
C12 VDDA 41 – 42, (44 – 45) 100 nF ±20% VDDA Ceramic X7R
C19, C20 XTAL 59 - 61, 60 - 61---    18 pF 2---±20% VDDPLL Ceramic NPO
X1 XTAL 59 - 60---4 MHz---±100ppm VDDPLL Quartz / Resonator
R20, R21 I2C pull-up  1 kOhm ±20% VDDIO Resistors
Dashed components improve signal quality, power supply quality or testability:
C1, C3 VDDPXH 46 – 47, 39 - 40  10 μF ±20% VDDPXH Ceramic X7R
C5, C7 VDDPXM 46 – 48, 38 - 40  10 μF ±20% VDDPXM Ceramic X7R
C13 VDDA 44 – 45  10 μF ±20% VDDA Ceramic X7R
C9 PIN43 41 - 43    1 μF ±20% VIR Ceramic X7R
C15 VBS 37 - 41    1 μF ±20% VBS Ceramic X7R
C21 VDDPLL 61 - 62    1 μF ±20% VDDPLL Ceramic X7R
C24, C26, C28 VDD 8 – 9, 28 – 29, 57 - 
58
    1 μF ±20% VDD Ceramic X7R
C30, C32, 
C34, C36
VDDIO 4 – 5, 12 – 13,
18 – 19, 66 - 67
    1 μF ±20% VDDIO Ceramic X7R
C2, C4 VDDPXH 46 – 47, 39 - 40 100 nF ±20% VDDPXH Ceramic X7R
C6, C8 VDDPXM 46 – 48, 38 - 40 100nF ±20% VDDPXM Ceramic X7R
C10 PIN43 41 – 43 100 nF ±20% VIR Ceramic X7R
C14 VDDA 44 – 45 100 nF ±20% VDDA Ceramic X7R
C16 VBS 37 - 41 100 nF ±20% VBS Ceramic X7R
C22 VDDPLL 61 - 62 100 nF ±20% VDDPLL Ceramic X7R
C23, C25, C27 VDD 8 – 9, 28 – 29, 57 - 
58
  10 nF ±20% VDD Ceramic X7R
C29, C31, 
C33, C35
VDDIO 4 – 5, 12 – 13,
18 – 19, 66 - 67
  10 nF ±20% VDDIO Ceramic X7R
L1 VDDPXM---100 nH ±20% VDDPXM Inductor
L2 PIN43---100 nH ±20% VIR Inductor
L3 VDDPLL---100 nH ±20% VDDPLL Inductor
R4 - R19 Bus
termination
0 Ohm 10 Ohm 33 Ohm ±20% VDDIO Resistors
R22, R23 I2C address 10 kOhm ±20% VDDIO Resistors
Table 11: Values of component related to epc660 chip, see Figure 17
Notes:
1 All other components are application specific.
2 The capacitor value has to be selected according the crystal or resonator supplier's recommendation.
5.3. Hardware implementation notes
1. epc660 is supplied with +1.8V, +2.5/3.3V, +5V, +10V and -10V. See Figure 17.
2. Decoupling capacitors must be placed as close as possible to their supply pin pair in order to minimize ripple on the supply rails due to
fast switching high-speed signals (Table 11).
3. +1.8V is used for supplying the digital logic (VDD), the on-chip oscillator OSC and the phase-look-loop PLL (VDDPLL). These supplies
are marked in the application diagram as VDD and VDDPLL respectively (Figure 17). Their supply wiring must be separated from the
digital wires and physically isolated from each other. The XTAL/OSC and PLL are critical parts of the chip which directly impacts the
optical system performance (i.e. distance calculation). Thereof, the VDDPLL supply needs a well decoupling from VDD, because the
digital logic creates some internal switching noise on VDD. 
4. +2.5/3.3V (VDDIO) is used for supplying the high-speed IO pins (MODCLK, TCMI and LED2) and the slow I2C pins. High speed TCIM
pins toggle up to 48MHz during data transfer, hence generating continuously switching noise (much more dominant than the digital
noise). Therefore, VDDIO supply wires and layers must be carefully designed and isolated in a separate supply island on the PCB. It is
not recommend to change this voltage on the fly when the TCMI, LED2 or I2C interfaces are running. When the application needs
power saving during system idle periods, it can be scaled from +3.3V down to +2.5V, only after frame acquisition is stopped and both
interfaces are completely inactivated. It can be increased back to +3.3V before re-activating the chip for frame acquisition, accessing
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
19 / 70 Datasheet_epc660-V2.20
www.espros.com
I2C, LED2 or TCMI interface. Note that voltage scaling must be done in a controlled way having both application CPU's and epc660's
IO voltages at the same time at the same level.
5. +5V is used for supplying analog blocks of the chip e.g. pixel-field drivers and ADC readout circuitry. Refer to Figure 17.
6. +10V (VDDPXH) is used for supplying the pixel-field circuitry.
7.-10V (VBS) is used for biasing the the pixel-field like reverse-biasing a photodiode. The use of a stable supply source with a low ripple
is recommended. There is no switching or active internal circuit dependent current consumption, except ambient-light dependent leak
age current (refer to Table 1, note 8).
8. A 4MHz quartz crystal or a ceramic resonator is connected to XTALIN_CLKIN and XTALOUT pins in order to use internal oscillator
OSC as time base for the epc660. The frequency accuracy and stability are directly related to the distance readings. Alternatively an
external clock source can be used (chapter 5.4).
9. MODCLK input can be used for user controlled/modulated clock. It is used for both the LED driver and the pixel-field demodulator.
10. SCL, SDA are I2C slave pins which need external pull-up resistors on the PCB (see also VDDIO supply). Values of R20 and R21 are
given only for indicative purpose and must be re-calculated according to the total capacitive load of all I2C slave/master devices and
the operating mode FM or FM+ of the I2C (chapter 13) in the application.
11. VSYNC_A0, HSYNC_A1, XSYNC_SAT, DATA[11:0], DCLK, high-speed TCMI signals (chapter 6), SHUTTER and RESET control sig
nals toggle in the VDDIO range. To minimize the skew, the high-speed *SYNC, DATA[11:0], DCLK signals wires must be routed equal
in impedance and length less than 10cm long with less than 10mm difference on the PCB. As they are toggling all the time, they can
be separated with ground wires on the side adjacent to other signals/supply lines, routed with enough distance from other sensitive
signal wires on the board. Series termination resistors R4 … R19 (10 … 33Ω) are needed at high-speed outputs to control the slew.
12. Optional pull-up resistors R22 and R23 (10kΩ) set initial values of some configuration registers during start up of the chip. Such out
puts pins are called strap pins. They are scanned one time immediately after RESET is released (chapter 5.6.3).
13. The LED pin is an open-drain LED/LD driver output. When the driver is active (on), the LED/LD on-current flows through the power
resistor R2 into the LED pin, through the driver and comes out of the chip on the VSSLED ground pin. The LED pin toggles up to 
24MHz or according to the MODCLK clock with a current maximum of 200mA limited by the resistor R2. The number of IR LEDs de
pends on the level of the LED supply voltage and the turned-on forward voltage drop of the IR LEDs. This signal creates a lot of
ground noise. Therefore, VSSLED pin is decoupled from the other analog grounds internally. It must be shorted with the other analog
ground pins with a low-ohmic connection as short as possible on the PCB. In this way, there will be minimal voltage differences in the
ground planes of the board. The LED supply line must be isolated properly from any analog supply on the PCB to minimize noise
coupling from the LED drivers. 
250
200
I_LED (mA)
150
100
50
0
0.0
0.1
VDD_LED (V)
0.2
Figure 18: Output characteristic ILED versus VDDLED.
Refer for maximum values of VDDLED and ILED to Table 1 and Table 2
14. The LED2 pin is the alternative push-pull driver providing symmetric rise/fall times to drive external LED driver. It works from the
+2.5/+3.3 VDDIO supply (VSSIO GND domain) and swings in the same voltage range like the TCMI pins. LED2 = LOW (approx. 0V)
corresponds to LED = OFF (max. output voltage). LED and LED2 pins must not be used at the same time for driving the external illu
mination. They exhibit different phase delays and this can result wrong distance measurements. None of the TCMI pads toggle during
integration time, LED2 pin is the only toggling during integration time and it is not affected from switching noise of others.
15. It is recommended having “not connected pins” (PINxx) on test pads available. It helps e.g. to check after assembly for correct orienta
tion of the chip or for short-cuts.
16. Pins not listed here have to be connected according Figure 17.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
20 / 70
Datasheet_epc660-V2.20
www.espros.com
5.4. Clock source
Instead of a crystal, an external 4MHz clock source can be connected to the XTALIN_CLKIN pin. XTALOUT output pin left unconnected.
Input clock signal levels must match VDDPLL/VSSPLL supply levels (Table 1). If the external clock source comes from the +2.5/3.3V volt
age domain, a resistor divider circuit can be deployed to adjust the voltage level according to Figure 19.
NC
OSC
PLL
R27
2k2/1k2
VDDIO
+2.5V/+3.3V
R26
1k/1k
External clock
XTALOUT
60
XTALIN
_CLKIN
59
VSSIO
VSSIO
epc660
Figure 19: Resistor divider to adjust external clock voltage levels to XTALIN_CLKIN
IMPORTANT: The optical performance of the chip directly depends on the input clock precision/stability. XTALOUT must not be used to
drive external loads.
5.5. External modulation MODCLK
The epc660 has for enhanced user applications the possibility to bring an external modulation clock to the chip. The optional MODCLK in
put can be used to inject a user controlled/modulated clock for both the LED driver and the pixel demodulator, see Figure 20.
The external MODCLK can be used e.g. in concepts for reliable multi camera applications. It allows to use e.g. frequency-division multiple
access (FDMA). In corresponding literature, the details of these concepts are explained in detail.
Memory
Controller
&
Registers
m+n
Clock Delay
Pixel 
Cont.
&
Seq. 
Memory
I2C Slave
TCMI 320x240
Pixel Field
SCL
SDA
VDD
VDDIO
XTALOUT
XTALIN_CLKIN
VDDPLL
PLL
VSSPLL
    
CGU
ADC Read-Out
ROW Buffer
Charge 
Pump 
13V
EEPROM
LED Driver
LED (open-drain)
 
 
Modulator
 
 
De
modulator
DATA[11:0]
DCLK
VBS
4x Temp.
Sensors
at corners
VSSLED
VSYNC_A0
VSSIO
VSS
ADC Read-Out
ROW Buffer
VDDPXH
epc660
OSC
VDDPXM
VDDA
VSSA
SHUTTER HSYNC_A1
VSSPX
XSYNC_SAT
MODCLK
RGU
RESET
DLL 0
1
LED2 (push-pull)
Figure 20: The MODCLK signal flow (red marked)
The user is free to apply any digital waveform up to 96MHz during frame acquisition as external MODCLK signal. Even more, he is also
free to use modulations like pseudo-random edge jitter, dithering, etc. 
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
21 / 70 Datasheet_epc660-V2.20
www.espros.com
The signal from the MODCLK pin is used instead of the clock generated by the CGU if bit 6 in register 0x80 is set to 1. The effective  mod
ulation signal is the MODCLK divided by 4. 
The unambiguity range and the integration time are in this case based on the MODCLK:
[1]
tINT 
= reg(0x85)+ 1
MODCLK ⋅[reg(0xA2:0xA3)+ 1]⋅reg(0xA0:0xA1)
For more details refer to chapter 9.2.1 and 11.4. Note, register 0x85 is active in this mode.
5.6. Supply, reset and start-up options
5.6.1. Supply voltages and external reset
During the power-up sequence, VDD and VDDPLL supplies (Figure 21) must be applied at the same time to the epc660. VDDIO can be
applied either at the same time or after VDD and VDDPLL supplies become stable. In a system where VDDIO voltage is connected in par
allel to application CPU IO supply pins (see Figure 17), VDD and VDDPLL can be generated by a linear regulator directly from VDDIO sup
ply. In this case, all these three supplies ramp together.
VDDA, VDDPXM and VDDPXH supplies must be applied as a second group, after all VDD, VDDPLL and VDDIO supplies become stable.
The negative supply VBS must be applied after all positive supplies reached their rated levels.
Image acquisition shall not start before all supply voltage are at their stable level.
RESET must be kept low while all positive voltages are ramping-up in order to guarantee proper reset of all internal circuits. As soon as
rated positive levels are reached, RESET can be set to high. In case of an external clock is applied at XTALIN_CLKIN instead of a crystal/
resonator is used with on-chip OSC, clock must be present before RESET is released.
IMPORTANT:
■ It is possible to shutdown entire supplies for a very low standby current. In that case, first RESET must be driven low, then supplies must
be turned off in the reverse order. Refer for details to chapter 11.5
■ VDDA, VDDPXM and VDDPXH supplies must never kept on while turning off VDD, VDDPLL and VDDIO. Damage to the chip can be the
result.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
22 / 70
Datasheet_epc660-V2.20
www.espros.com
tPositive_supply_ramp
tNegative_supply_ramp
tVDDIO_ramp
tVDDPX_ramp
tVDD_ramp
V
V
VDDPLL
V
VDDPLL
VDDPXH
VDDPXM
, VDDA
VDDIO
VDD
, VDDPLL
VBS
Oscillator OSC (XTALIN_CLKIN, XTALOUT)
pll_clk (PLL turn-on)
V
VDDIO
tPLL_lock
RESET
t
t
t
all positive
supplies stable
tRESET
V
VDDIO
tStrap_scan
tEEPROM_to_CFG_copy
t
SHUTTER
frame
acquisition
PLL locked (stable)
Figure 21: Power-up and reset sequence
5.6.2. Start-up (Clock, PLL turn-on and EEPROM copy)
t
all supplies stable
end of start-up
The epc660 starts using either the internal 4MHz oscillator OSC with a crystal/resonator (Figure 17) or an external 4MHz clock, followed by
an EEPROM copy sequence in parallel to the PLL turn-on phase. This is the factory default configuration. Several configuration registers
are modified by copying the EEPROM content (Figure 58, i.e. overwrite reset values).
5.6.3. Strap pins
The epc660 has output pins with dual/alternative functionality for PCB level flexible start-up configuration changing, called 'strap pins'. 
RESET release is followed by a strap pin scanning step. The chip programs its strap pins as inputs with internal pull-down resistors en
abled for 4 osc_clk periods (refer to Table 1 and Table 3.). If there is no external pull-up resistor connected, the corresponding strap pin will
be scanned as logic 0 due to the internal pull-down resistor. If there is an external pull-up resistor connected (Figure 17), it will override the
internal pull-down and corresponding pin will be scanned as logic 1. After the strap scan period, pins are programmed back as outputs so
that they can be used for their main function. Strap pins and their definitions are listed below (Table 12).
Pin
Pin no.
Definition
HSYNC_A1
64
Set A1 bit of 7-bit I2C slave device address (section 13.1).
VSYNC_A0
63
Set A0 bit of 7-bit I2C slave device address (section 13.1).
XSYNC, DATA0
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
1, 2
Factory used strap pins. No pull-up resistors allowed
Table 12: Strap pin definition
23 / 70
Datasheet_epc660-V2.20
www.espros.com
5.7. LED driver
The LED driver register 0x90 is used for setting polarity etc. depending on the external LED/LD circuitry used in the application. These bit
fields must not be modified during frame acquisition.
IMPORTANT: There are non-modulating DC modes (e.g. grayscale with LED/LD illumination) which keeps the LED driver always turned
on. In this case, the user has to take care that LED driver and the epc660 chip does not exceed the maximum operating limits.
5.8. DLL (Delay Line)
The modulation signal can intentionally be delayed in order to add a phase shift between the modulation of the light source and the de
modulation of the backscattered light, refer to Figure 22.
tPix
Modulator
clock
n*t DLL
n*t DLL
Pixel field
tTOF
DLL disabled
Coarse DLL
stages from
0 to 49
(Reg. 0x73)
Fine DLL
stages
(Reg.
0x71/72)
DLL enabled 
(Reg. 0xAE)
tOD
LED
LED2
Illumination
Illumination
driver
Figure 22: Block diagram of the DLL function
The purpose to do so can be that the phase shift between the modulated and the demodulated signal in a specific distance range should
be at a certain value. For example, the highest distance accuracy with lowest distance noise can be achieved when the phase angle of de
modulation is 45°. This is the case when all four DCS amplitudes have the same or a similar value. The worst situation is if one DCS pair is
at its maximal amplitude whereas the other DCS pair is around zero (refer to Figure 23).
The DLL can be enabled in register 0xAE whereas the delay of the LED modulation can be set in steps tDLL by register 0x73 (approx. 2ns/
step). The exact step tDLL by can be calculated with the value and the formula listed in register 0xE9. This value is varying from chip to chip
and is also temperature dependent. The user shall characterize the overall temperature drift of the complete camera for matching the com
pensation.
Figure 23: DCS amplitudes for the 4 DCSx (measurement data)
Example for 10MHz modulation frequency:
If we want to optimize the accuracy of our TOF camera in the short range domain, e.g. 0m to 1m, the situation shown in Figure 23 is not
ideal at all. The modulation frequency of the data shown in Figure 23 is 10MHz whereas 50 DLL Steps of approx. 2ns are equivalent to
15m distance. Shown in the diagram, the worst condition is in the first three DLL steps, which is equal to 0m to 0.9m. From then on, the
distance accuracy becomes much better until DLL step 12. In other words, the distance accuracy from distance 0.9m to 3.0m is very good,
but not from 0m to 0.9m. In order to be in an accurate distance measurement regime, the DLL should be shifted by 3 steps which means
that the LED is delayed by 6ns.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
24 / 70
Datasheet_epc660-V2.20
www.espros.com
5.9. Application system overview
Figure 24 and Figure 25 show a typical application block and data flow diagram. The epc660 chip acquires image data, controlled via the
I2C interface, and then submits the data via the TCMI to an FPGA or microcontroller. The FPGA or microcontroller calculates the distance
from the DCS and does filtering, correction and compensation and provides a cleaned “point cloud” to the host system.
Illumination
CUSTOMER's
FPGA          
epc660
Shutter
TCMI
Configuration
Registers
0..20
S
M
Commands,
Start/Stop
Sequence
e.g. 16b Parallel Interface
I2C
Image Capture
4 DCS
Processing the 4 DCS
in FPGA:
Distance calculation
Confidence calculation
FPN correction
Temperature compensation
S
M
M
CUSTOMER's
Host System
Figure 24: Block diagram
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
4th order compensation
Amplitude compensation
I2C
e.g. Distance & Confidence
Figure 25: Data flow
25 / 70
Datasheet_epc660-V2.20
www.espros.com
6. TOF camera interface (TCMI)
The TOF Camera Module Interface (TCMI) is a programmable high-speed parallel data output interface to down-load the pixel data. It can
be programmed very flexible via the registers 0x89, 0xCB and 0xCC.
When the integration period is completed and ADC conversion is finished, the readout results are moved into the data out buffers to be im
mediately transmitted via the TCMI interface. The ADC conversion is two full rows in parallel (top and bottom pixel-field) and the conver
sion time is independent of the number of selected columns. Depending of the mode selection (4x DCS, 2x DCS, ...) a programmable
number of DCS frames are generated. The data is streamed out as a complete block of 1 DCS frame, one after the other. Each row con
tains 12-bit DCS values and the SAT bit. The pixel values are streamed out as 12 bit signed numbers.  Two rows are streamed out in se
quence together, the first one from the top and the second one from the bottom pixel-field e.g. R125 (C4, C5, … C323), R126 (C4, C5, …
C323), R124 (C4, C5, … C323), R127 (C4, C5, … C323) and so on until R6 (C4, C5, … C323), R245 (C4, C5, … C323). The stream-out
of a row pair takes 26.7µs with default clock settings (24MHz TCMI clock rate).
The transfer of a DCS frame cannot be interrupted or stopped, once it is started. The application should have enough bandwidth to receive
all transmitted frames.
IMPORTANT:
6.1. TCMI clock
Refer to register 0xCC for setting correct data format.
The TCMI interface supports the continuous clock mode with DCLK signal toggling continuously. It transmits the frames at high-speed us
ing all *SYNC (VSYNC_A0, HSYNC_A1, XSYNC_SAT), DATA[11:0] and DCLK outputs (Figure 26). The DCLK frequency is programmable
to 12, 24, 48 MHz via register 0x89.
Measurement cycle with selected number of DCS frames
SHUTTER
Frames
Init
Integration
Processing
Conversion
R125,126 even
Conversion
R125,126 odd
Data out
R125
Data out
R126
Conversion
R123,128 even
Data out
R6
Data out
R244
Init
Integration
Conversion
R124,127 even
VSYNC
Conversion
R124,127 odd
Data out
R124
Data out
R127
Temperature
sensor update
HSYNC
Conversion
R6, 245 even
Conversion
R6, 245 odd
Conversion
R6 odd
Data out
R6
Data out
R245
Temperature
sensor update
DATA[11:0] & SAT
Blanking
Valid data
Blanking
Valid data
DCLK
Continuous clock: DCLK is running all the time
Valid data
Blanking
Valid data
1st DCS frame of the measurement cycle
Last DCS frame of the measurement cycle
Figure 26: Continuous clock mode
All *SYNC*, DATA[11:0] signals are synchronously updated with the positive edge of the DCLK signal when its polarity is set as active
high; with the negative edge of the DCLK signal when its polarity is set as active-low. The non-active edge of the DCLK output can be used
by the receiving end (application CPU) as a sampling clock. It should approximately be in the center of the data (refer to Figure 28). By us
ing the default configuration, the active states of VSYNC_A0 and HSYNC_A1 signals indicate blanking periods during the frame transmis
sion. While DCLK toggles continuously, any data during the blanking periods are not valid and must be ignored.
As soon as the measurement result of the first row of the new frame is available, VSYNC_A0 and HSYNC_A1 are set consecutively with 
the next active edge of DCLK. VSYNC_A0 is active from the start until the end of the each complete frame. Whereas, HSYNC_A1 indi
cates the validity of the DATA[11:0] and XSYNC_SAT (saturation bit) from the start until the end of a row pair.
By default, the XSYNC_SAT pin is used for the saturation bit. Optionally, it can be programmed to indicate the end of a frame by disabling 
bit 6 in register 0xCC.
6.2. Single or continuous measurement control
6.2.1. Single measurement control
The selected measurement mode (4x DCS, 2x DCS, grayscale, …) defines, how many frames the chip performs by the stimulation of one
SHUTTER pulse for a measurement cycle. This pulse can be applied either by the HW SHUTTER pin or by SW control with bit 0 in regis
ter 0xA4. Whereas the SW controlled SHUTTER is auto-cleared after propagation, the HW Shutter needs a minimum hold time of 250ns
and must be set back manually latest before the HSYNC_A1 signal of the last row pair of the last DCS frame (last HSYNC_A1 of the last
frame). During such a measurement cycle, the next frame acquisition starts immediately after the last data readout on the TCMI interface
until all frames are performed.
6.2.2. Continuous measurement control (auto-run)
As long as in the shutter control register 0xA4, bit 1 is set or the HW SHUTTER is applied during the readout of the last row pair of the last
frame, the epc660 runs in a non-stop measurement mode. The chip starts immediately next measurement cycle if the actual one is termi
nated (Figure 30). Trigger signals not active during the readout of the last row pair of the last frame are ignored.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
26 / 70
Datasheet_epc660-V2.20
www.espros.com
6.3. TCMI timing
DCLK
tL
tH
t rfDCLK
t rfDCLK
0.8 x VDDIO
0.2 x VDDIO
tDCLK
VSYNC
HSYNC
Data out Data out Data out Blanking
tDDV
DATA [11:0]
XSYNC_SAT
tDDC
tData valid
tDV
Blanking
Figure 27: Detailed TCMI timing
Symbol Parameter Min. Typ. Max. Units
tDCLK TCMI readout clock: typ. fDCLK = 24MHz / max. fDCLK = 48MHz 41.6 20.8 ns
tDDV Delay time after positive edge of DCLK until data are valid 2.0 ns
tDDC Data start changing before positive edge of DCLK 1.7 ns
trfDCLK Rise and fall time of DCLK, VSYNC, HSYNC, XSYNC, Data[11:0] 2.0 ns
tH High period of DCLK 5.0 ns
tL Low period of DCLK 3.5 ns
tData valid Output data on the TCMI interface are valid (depends on DCLK) 8.8 ns
Table 13: TCMI timing parameters (CL = 20 pF max.)
R125
C4
1 2 3 4 5 6 7 8
TPLL
R125
C5
R125
C6
R125
C7
VSYNC
HSYNC
DATA[11:0]
XSYNC_SAT
DCLK
pll_clk
TCMI detailed bus timing: DCLK=48MHz (pll_clk / 2)
R125
C4
1 2 3 4 5 6 7 8
TPLL
R125
C5
R125
C6
VSYNC
HSYNC
DATA[11:0]
XSYNC_SAT
DCLK
pll_clk
TCMI detailed bus timing: DCLK=32MHz (pll_clk / 3)
R125
C4
1 2 3 4 5 6 7 8
TPLL
R125
C5
VSYNC
HSYNC
DATA[11:0]
XSYNC_SAT
DCLK
pll_clk
TCMI detailed bus timing: DCLK=24MHz (pll_clk / 4)
R125
C4
1 2 3 4 5 6 7 8
TPLL
R125
C6
VSYNC
HSYNC
DATA[11:0]
XSYNC_SAT
DCLK
pll_clk
TCMI detailed bus timing: DCLK=19.2MHz (pll_clk / 5)
Figure 28: TCMI timing examples with symmetric and asymmetric DCLK
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
27 / 70 Datasheet_epc660-V2.20
www.espros.com
Con...
Dat...
Frames
SHUTTER
Continuous clock: DCLK is running all the time
VSYNC
HSYNC
DCLK
Blanking Valid data Blanking
Conversion even
R125, R126
Data out
R125
Conversion odd
R125, R126
Data out
R126
Conversion even
R124, R127
Conversion odd
R124, R127
Integration Init
DATA[11:0] & SAT
1st DCS frame of the measurement cycle
Processing
tSHUTTER
 
t INT
tPROC
 tCONV
 
tHSYNC
 
tSHUTTER_lag
 
Figure 29: Frame timing: Start 1st DCS frame
Con...
Dat...
Frames
SHUTTER
Continuous clock: DCLK is running all the time
VSYNC
HSYNC
DCLK
Blanking Valid data Blanking 
Conversion even
R125, R126
Data out
R125
Conversion odd
R125, R126
Data out
R126
Conversion even
R124, R127
Conversion odd
R124, R127
Data out
R6
Data out
R245
Conversion odd
R6, R245
DATA[11:0] & SAT
SHUTTER can be kept always “high” or to be applied during last data out for having maximum frame rate
nth DCS frame
next DCS frame
Integration Init Processing Con... Processing
tHSYNC_lag
 
tVSYNC_lag
 
tVSYNC
 
tFRAME_continuously
Figure 30: Frame timing: Inter frame timing, end of frame and start next frame
Note: 
To avoid readout rollover when using slower DCLK with default ROI (< 13MHz, register 0x89 > 0x06), register 0x91, bit 6 must be enabled.
It stretches HSYNC for slower TCMI interfaces. It causes a reduced DCS frame rate due to additional 2µs per ADC conversion (tconv + 2µs).
6.4. TCMI data format
TCMI supports one 12 bit and three 8 bit transfer formats: 
■12-bit mode: Transfers 12 bit pixel data with 1x DCLK (default).
Refer to Figure 31.
■msb/lsb split mode: Transfers 12 bit pixel data with MSByte leading and LSByte trailing with 2x DCLK.
Refer to Table 14 and Figure 32.
■lsb/msb split mode: Transfers 12 bit pixel data with LSByte leading and MSByte trailing with 2x DCLK.
Refer to Table 15 and Figure 33.
■8-bit mode: Transfers the 8 MSB bits of the pixel data with 1x DCLK.
Refer to Table 16 and Figure 35.
12-bit mode uses all lines DATA[11:0]. Whereas the three 8-bit modes require only lines DATA[7:0] to be connected in the application. The
TCMI data format can be selected in the register 0xCB.
The two split modes transmit pixel values in two consecutive DCLK cycles. As a result HSYNC time is doubled. When 8 bit precision is
enough, the application can use 8-bit mode.
1st Byte: MSByte 2nd Byte: LSByte
D7 D6 D5 D4 D3 D2 D1 D0 D7 D6 D5 D4 D3 D2 D1 D0
SAT 0 0 0 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
Table 14: TCMI msb/lsb split mode
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
28 / 70 Datasheet_epc660-V2.20
www.espros.com
1st Byte: LSByte 2nd Byte: MSByte
D7 D6 D5 D4 D3 D2 D1 D0 D7 D6 D5 D4 D3 D2 D1 D0
b7 b6 b5 b4 b3 b2 b1 b0 SAT 0 0 0 b11 b10 b9 b8
Table 15: TCMI lsb/msb split mode
Byte
D7 D6 D5 D4 D3 D2 D1 D0
b11 b10 b9 b8 b7 b6 b5 b4
Table 16: TCMI 8-bit mode , HW synchronization data format
The saturation flag can be optionally inserted into the DATA[7] of the MSByte by setting bit 6 in register 0xCB during the first or second
DCLK cycle for the msb/lsb or lsb/msb split modes, respectively. This feature is not available for the 12-bit and 8-bit mode. In this cases ei
ther the XSYNC_SAT pin can be used along with the DATA[*] pins or bit 7 in register 0xCC must be set to force all DATA[*] = 0xFFF when
the corresponding pixel is saturated.
BLANKING
tFrame
DATA[11:0]
DCLK
HSYNC
VSYNC
BLANKING R125
C4
R125
C5
R125
C323
R126
C4
R126
C5
R126
C323
R245
C4
R245
C5
R245
C323 BLANKING
t row top pixel field
t row bottom pixel field
Figure 31: 12-bit mode data readout
tFrame
DATA[7:0]
DCLK
HSYNC
VSYNC
BLANKING BLANKING BLANKING BLANKING
1. 2. 1. 2. 1. 2. 1. 2. 1. 1. 1. 2. 1. 2. 1. 
R125
C4
[11:8]
R125
C4
[7:0]
R125
C5
[11:8]
R125
C323
[11:8]
R125
C323
[7:0]
R126
C4
[11:8]
R126
C4
[7:0]
R126
C5
[11:8]
R126
C323
[11:8]
R126
C323
[7:0]
R245
C4
[11:8]
R245
C4
[7:0]
R245
C5
[11:8]
R245
C323
[11:8]
R245
C323
[7:0]
Figure 32: msb/lsb split mode
BLANKING
R125
C5
[7:0]
R125
C323
[7:0]
R125
C323
[11:8]
R125
C4
[11:8]
R125
C4
[7:0]
R126
C5
[7:0]
R126
C323
[7:0]
R126
C323
[11:8]
R126
C4
[11:8]
R126
C4
[7:0]
R245
C5
[7:0]
R245
C323
[7:0]
R245
C323
[11:8]
R245
C4
[11:8]
R245
C4
[7:0]
BLANKING BLANKING BLANKING
1. 2. 1. 2. 1. 2. 1. 2. 1. 1. 1. 2. 1. 2. 1. 
 tFrame
DATA[7:0]
DCLK
HSYNC
VSYNC
Figure 33: lsb/msb split mode
tFrame
DATA[7:0]
DCLK
HSYNC
VSYNC
BLANKING
R125
C6
[11:4]
R125
C323
[11:4]
R125
C323
[11:4]
R125
C5
[11:4]
R125
C4
[11:4]
R126
C6
[11:4]
R126
C323
11:4]
R126
C323
[11:4]
R126
C5
[11:4]
R126
C4
[11:4]
R245
C6
[11:4]
R245
C323
[11:4]
R245
C323
[11:4]
R245
C5
[11:4]
R245
C4
11:4]
BLANKING BLANKING BLANKING
Figure 34: 8-bit mode
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
29 / 70 Datasheet_epc660-V2.20
www.espros.com
6.5. Frame rate and data-out performance
6.5.1. Frame rate QVGA 320x240 pixel (default)
The epc660 can perform a maximum of 158 fps with 1µs integration time, 12MHz modulation clock, 48MHz DCLK, 1x DCS and continuous
measurement control. For 3D TOF, each frame is referred as a DCS frame. Either 4x (with π-delay matching) or 2x (without π-delay
matching) DCS frames must be acquired for one distance calculation. Therefore, the resulting distance measurement rate turns out to be
39 fps or 79 fps respectively. For the grayscale mode the maximum frame rate of 158 fps is possible.
Symbol Parameter Min. Typ. Max. Units
tDCLK TCMI readout clock e.g. fDCLK = 48MHz 20.8 ns
tSHUTTER Hold time for the signal on pin SHUTTER 250 ns
tSHUTTER_lag Delay from the rising edge of SHUTTER signal to the 1st LED pulse 18 µs
tINT Image acquisition (integration time) 1 µs
tPROC Delay from the last LED pulse until the 1st row conversion 38.75 µs
tCONV Conversion time for a pair of half rows (even or odd) 26.042 µs
tHSYNC Readout time for a pair of rows e.g. fDCLK = 48MHz 13.33 µs
tHSYNC_lag Delay from the begin of last readout until the 1st LED pulse of next DCS frame 17 µs
tVSYNC_lag Delay end of HSYNC to end of VSYNC at the end of each DCS frame 50 ns
tVSYNC Data readout time for one DCS frame e.g. fDCLK = 48MHz
tVSYNC = (2x tCONV x 119 rows) + tHSYNC + tVSYNC_lag 
6'261 µs
Single measurement control mode:
t1st_FRAME_START Delay from rising edge of SHUTTER signal until start of data readout of 1st frame 83.79 µs
t1st_FRAME_TOTAL Total time for reading one DCS or grayscale frame from rising edge of 
SHUTTER signal until end of readout of 1st frame
6'345 µs
Continuous measurement control mode:
tFRAME_continuously Total time for reading one DCS or grayscale frame
tFRAME_continuously = (2x tCONV x 120 rows) + tHSYNC_lag + tINT + tPROC 
6'307 µs
t4DCS_continuously Total time for one 3D TOF distance measurement (4 DCS) 
tFRAME_continuously = ((2x tCONV x 120 rows) + tHSYNC_lag + tINT + tPROC ) x 4 DCS
25.23 ms
Table 17: Timings for one DCS or grayscale frames and for 3D TOF distance measurements (4x DCS)
(Reference: see Figure 29 and Figure 30, fDCLK = 48MHz, tINT = 1µs)
Ref. Imager settings (Input) Imager output
Figure Pixel-field mode Binning
hor., ver., both
Row reduction
y-axis: 2, 4, 8
Resolution x-y
[imager pixel]
DCS Frame rate
[fps]
Frame size 3
[kbytes]
42 full resolution no 1      320 x 240   158 150
42 full resolution no 2      320 x 120   314  75
42 full resolution no 4      320 x   60   617  38
42 full resolution no 8      320 x   30 1'119  19
43 full resolution horizontal 1      160 x 240   314  75
43 full resolution horizontal 2      160 x 120   617  38
43 full resolution horizontal 4      160 x   60 1'119  19
43 full resolution horizontal 8      160 x   30 2'235  10
44 full resolution vertical 2      320 x 120   314  75
44 full resolution vertical 4      320 x   60   617  38
44 full resolution vertical 8      320 x   30 1'119  19
45 full resolution both 2      160 x 120   617  38
45 full resolution both 4      160 x   60 1'119  19
45 full resolution both 8      160 x   30 2'235  10
46 dual modes 1 no 2 1 2 x 320 x 120   158 150
46 dual modes 1 no 2 2 2 x 320 x   60   314  75
46 dual modes 1 no 2 4 2 x 320 x   30   617  38
Table 18: Frame rate and resolution for default ROI setting 320 x 120 pixel, top-left (4, 6) and bottom-right (323, 125)
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
30 / 70 Datasheet_epc660-V2.20
www.espros.com
Notes:
1 Frame rate and frame size are identical for dual phase and dual integration time mode (dual modes).
2 Binning cannot be used with dual phase and dual integration time mode.
3 Frame size is based on 2 Bytes per pixel to store in the application frame buffer.
6.5.2. Frame rate Half QQVGA 160x60 pixel
This example shows the ROI set symmetrically to 2 x 160 x 30 (Half-QQVGA) in the middle of the pixel-field: epc635 emulation. The frame
time scales linearly with the reduced number of rows readout (see Table 19). The TCMI data-out time scales linearly with the reduced num
ber of columns set in the ROI.
Top pixel field
Bottom pixel field
Top-left
 (84, 96)
Bottom-right 
(243, 125)
(243, 126) ROI
(symmetric)
R6
7
8
9
C4
5
6
7
322
C323
R125
R126
127
242
243
R245
124
244
(84, 155)
320
321
ROI
Half-QQVGA ROI in the center
2x 160x30 pixel
Figure 35: ROI for Half-QQVGA: 2 x 160 x 30 pixel
Ref. Imager settings (Input) Imager output
Figure Pixel-field mode Binning
hor., ver., both
Row reduction
y-axis: 2, 4, 8
Resolution x-y
[imager pixel]
DCS Frame rate
[fps]
Frame size 2
[kbytes]
42 full resolution no 1      160 x 60   617 19
42 full resolution no 2      160 x 30 1'119 10
43 full resolution horizontal 1        80 x 60 1'119 10
43 full resolution horizontal 2        80 x 30 2'235  5
44 full resolution vertical 2      160 x 30 1'119 10
45 full resolution both 2        80 x 30 2'235  5
46 dual modes 1 no 1 1 2 x 160 x 30   617 19
Table 19: Frame rate and resolution for ROI setting: 160 x 30 pixel: top-left (84, 96) and bottom-right (243, 125)
Notes:
1 Binning cannot be used with dual phase and dual integration time mode.
2 Frame size is based on 2 Bytes per pixel to store in the application frame buffer.
6.5.3. Memory space estimation QVGA
Every frame (DCS) generates up to 320 x 240 pixel x 13 bit (Data + SAT) = 999 kBit. Stuffed to 16 bit words, the memory needed to store
one DCS frame is 154kByte. Depending on the operation mode, up to 10 full frames or even more are needed. Thus, the minimum image
memory RAM should be 1.5 MByte.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
31 / 70 Datasheet_epc660-V2.20
www.espros.com
7. Pixel-field and architecture
The pixels are placed in groups 2x2 pixels, called herein “pixel group”. The pixel group performs two basic operations: Measurement (inte
gration) and readout (ADC). Pixels are named as UE (Upper-row, Even-column), UO (Upper-row, Odd-column), LE (Lower-row, Even-col
umn) and LO (Lower-row, Odd-column) depending on their location within the pixel group (see Figure 36). Pixels with the same name are
controlled simultaneously in the whole pixel-field. More precisely, pixels in the upper and lower rows are controlled simultaneously during
measurement, pixels in the even and odd columns are controlled simultaneously during readout.
The pixel group architecture allows the epc660 to operate the pixel-field in different modes and in combinations thereof according the fol
lowing chapters.
t
C4
5
R6
7
Int_len
led_mod
mga0
UE
UO
mgb0
LE
LO
DCS0: mga 0°; mgb 180°
saturation detection
e
e
e
A (UE)
SGA
SGA
saturation detection
e
B (UE)
A - B
SAT
A
+
ADC
B
SGB
Integration
SGB
Difference SGA - SGB and saturation detection
Figure 36: The 2x2 pixel group and the simplified function overview
Digital read-out
SAT
12-bit
Data
Each pixel of the pixel group has its own pair of storage gates SGA and SGB. During the integration time, they accumulate the charges
(e-) created by the reflected modulated light coming from the object (see section 9, Imaging). They are controlled by the mga and mgb de
modulation signals. After the measurement is finished, the readout phase starts. The charges stored in the storage gates SGA and SGB
are read out as a difference A – B (ambient-light suppression) and converted into a single 12-bit digital value and a 1 bit saturation flag.
The output value can be either positive or negative depending on the demodulated phase and the offset of the signal chain.
7.1. Pixel coordinates
The epc660 pixel-field consists of a total of 328 x 252 pixels whereas 320 x 240 are active. 4 rows top/bottom and 6 columns left/right on
the periphery of the pixel-field contain dummy pixels. The upper-left corner (top view on chip) is the origin (4/6) of the epc660 pixel-field. X
axis starts at 4 and counts up to 323 to the right. Pixel y-axis starts at 6 and counts up to 245 to the bottom. All readout modes and control
registers use this coordinate system to set or change modes of the chip.
The pixel-field is split vertically into top and bottom. The data read-out is in parallel top and bottom to double the frame rate. It starts in the
middle of the row axis. Thus the higher the row number the more dark current is collected by the pixels which appears like an increased
DC offset of the pixel value (refer to chapter 1.4). The internal readout of a row is split in two sections: first all even pixels; second all odd
pixels. Later on the TCMI interface presents the row in the regular order with even and odd pixels mixed.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
32 / 70
Datasheet_epc660-V2.20
www.espros.com
E E
EOEO
E E
EOEO
dummy pixels
Top
Pixel Field
Bottom
Pixel Field
0
1
2
3
4
5
R6
7
8
9
0
1
2
3C4
5
6
7
322
C323
324
325
326
327
R125
R126
127
128
129
246
247
248
249
250
251
R245
124
244
effective columns
effective rows
Data out buffer: even & odd pixels; 320 x (12 bit DATA + SAT flag) EOEO EO
ADC: even or odd pixels; 160 x (12 bit DATA + SAT flag) E E E
pixel
coordinates x
y temp. sensor
top-right
temp. sensor
bottom-right
temp. sensor
top-left
temp. sensor
bottom-left
dummy pixels
dummy pixels
dummy pixels
odd
even
E E
EOEO
E E
EOEO Data out buffer: even & odd pixels; 320 x (12 bit DATA + SAT flag) EOEO EO
ADC: even or odd pixels; 160 x (12 bit DATA + SAT flag) E E E
odd
even
readout rows
readout columns
readout columns
readout rows
Figure 37: Pixel-field coordinates with row and column numbering scheme (top-view, solder balls are bottom side)
7.2. Pixel saturation detection
The pixels collect continuously modulated and non-modulated ambient light during the integration period. Depending on these light intens
ities, the pixels may collect more charge (over-exposure) than they can accommodate in their storage gates (refer to Figure 36). In such a
case, the 12 bit sample data is not valid and cannot be used for distance calculation.
7.2.1. Hardware saturation flag
Each pixel generates a “saturation detection” flag along with the sample data, so that the data can be discarded by the application. The
saturation flag is transmitted via XSYNC_SAT pin with every pixel.
7.2.2. Software saturation flag
If XSYNC_SAT pin is used for an another function by setting register 0xCC, bit 6, bit 7 in register 0xCC enables to drive all DATA[11:0] to
0xFFF when the pixel is saturated.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
33 / 70 Datasheet_epc660-V2.20
www.espros.com
8. Operation modes
8.1. Full resolution mode (default)
This is the default operation mode for 3D TOF operation. All UE, UO, LE, LO storage gates work simultaneously during measurement op
eration. The storage gate control signals mga, mgb are applied to all pixels simultaneously (see Figure 38). One, two or four DCS can be
acquired in this mode.
UE UO
LE LO
DCS0: mga0 0°, mgb0 180°
R6
7
C4
5
led_mod
mga0
mgb0
t Int_len
R6
7
C4
5
8
9
6
7
8
9
10
11
10
11
12
13
320x240 pixel
Pixel field:
Figure 38: Full resolution mode: even and odd pixel rows are controlled identically with mgx0
8.2. Dual phase mode (motion blur reduction)
In this mode, the odd and even rows are controlled by 90° phase shifted signals (see Figure 39). This mode allows to acquire two 90°
shifted DCSs at the same time, e.g. DCS0 and DCS1. In the two-DCS mode, distance calculation can be accomplished within one acquisi
tion. Thus, motion blur is eliminated. The even row pixels store DCS0 (or DCS2) while the odd row pixels store DCS1 (or DCS3). The verti
cal pixel pairs (e.g. UE/LE) must be treated for distance calculation as if they are one single pixel. This comes at the cost of a reduced res
olution along the y-axis. The result provides a total of 320x240 pixel-field readout with an effective 3D TOF resolution of 240x120 pixel. It is
worth mentioning that the two middle rows have the same phase and it alternates from there.
Select this mode according chapter 11.4.1.
R6
7
C4
5
mga0
mgb0
mga1
mgb1
DCS 0
DCS 1
DCS 0
DCS 1
UE UO
LE LO
UE UO
LE LO
led_mod
mga0
mgb0
mga1
mgb1
t Int_len
DCS0: mga0 0°, mgb0 180°
DCS1: mga1 90°, mgb1 270°
R6
7
C4
5
8
9
6
7
8
9
10
11
10
11
12
13
2x 320x120 pixel
Pixel field:
Figure 39: Dual phase mode with phase-shifted integration time:
even and odd rows independently controlled by mgx0 and mgx1 with different phase shifts
IMPORTANT: This mode requires that adjacent pixels look to the same point on the target and receive the same amount of light.
Otherwise, calculated distance values are not reliable.
Pixel with a big offset or defective pixel will lead to completely wrong distance values with its paired pixel. 
Thus, the pixel group has to be discarded.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
34 / 70 Datasheet_epc660-V2.20
www.espros.com
8.3. Dual integration time mode (high dynamic range, HDR mode)
In this mode, the odd and even rows are controlled by different integration time lengths. It allows to acquire one image with two different in
tegration times in order to increase the dynamic range. Both groups provide exactly the same DCS modulation signals (phases). One
stops earlier than the other due to different integration times (see Figure 40). As a consequence, the two pixels collect different amount of
light simultaneously. There is no restriction about which integration time is shorter or longer with respect to the other. The even row pixels
integrate with integration length 1, register 0xA2 and 0xA3 while the odd row pixels integrate with integration length 2, register 0x9E and
0x9F. At the transition between the upper and lower pixel field are two columns with the same integration time. The even and odd pixels
(e.g. UE, LE) must be used independently for distance calculation. Finally, the vertical pixel pairs (e.g. UE/LE) must be treated as if they
are one single pixel by using only the better of the two pixel signals. This comes at the cost of a reduced resolution along the y-axis. In
stead of one frame with 320x240 pixels, a single readout provides two DCS or black and white frames with an effective resolution of
320x120 pixels but with different integration times.
Select this mode according chapter 11.4.2.
IMPORTANT: Crosstalk will occur if there is a large difference between the selected integration times.
We recommend not to go beyond the factor of 5.
R6
7
C4
5
mga0
mgb0
mga1
mgb1
UE UO
LE LO
UE UO
LE LO
t Int_len
t Int_len_mgx1
led_mod
mga0
mgb0
mga1
mgb1
DCS0: mga0 0°, mgb0 180°
DCS0: mga1 0°, mgb1 180°
R6
7
C4
5
8
9
6
7
8
9
10
11
10
11
12
13
2x 320x120 pixel
Pixel field:
DCS 0
DCS 0
DCS 0
DCS 0
Figure 40: Dual integration time mode:
even and odd rows independently controlled by mgx0 and mgx1. One stops earlier than the other.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
35 / 70 Datasheet_epc660-V2.20
www.espros.com
8.4. Pixel binning
The charges accumulated in the storage gates during integration can be combined by binning: horizontal, vertical or both (see Figure 41). 
IMPORTANT: Increases pixel to pixel noise, only internally recommended
Offers higher sensitivity, reduced integration time and faster readout of frames
Binning requires corresponding resolution reduction being enabled the same time. Refer to register 0x94.
Binning cannot be used with dual phase and dual integration time mode.
R6
7
C4
5
horizontal
binning
vertical
binning
horizontal+vertical
binning
UE UO
LE LO
UE UO
LE LO
UE UO
LE LO
Pixel group:
no
binning
UE UO
LE LO
00 01 01 Row reduction = 
01 00 01 Column reduction = 
01 10 11 Pixel binning = 00
00
00
160 x 240 320 x 120 160 x 120 Reduction of pixel field resolution: 320 x 240
2 x 2 x 4 x Increase of sensitivity: 1 x
C4
5
6
7
8
9
10
11
C4
5
6
7
8
9
10
11
C4
5
6
7
8
9
10
11
C4
5
6
7
8
9
10
11
R6
7
8
9
10
11
12
13
R6
7
8
9
10
11
12
13
R6
7
8
9
10
11
12
13
R6
7
8
9
10
11
12
13
Pixel field:
Mode setting:
Register 0x94:
2 x 2 x 4 x Increase of DCS frame rate: 1 x
√2x Increase of ratio signal to shot-noise: 1 x √2x 2 x
1/2 x 1/2 x 1/4 x Decrease of DCS frame readout time: 1 x
Figure 41: Pixel binning modes and readout
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
36 / 70 Datasheet_epc660-V2.20
www.espros.com
8.5. Resolution reduction
Resolution reductions by reading only every 2nd column on x-axis and every 2nd , 4th and 8th row on y-axis are supported independently. It
can be combined with binning (see chapter before), ROI (see next chapter), motion blur reduction and high dynamic range modes. See
Figure 42 - Figure 47 for example combinations.
IMPORTANT: Dual phase and dual integration time modes can be used with resolution reduction only, not with binning. 
Resolution reduction shrinks the dataset to the necessary amount of data required for the application. The advantages are the reduced
amount of data to be processed for the final measurement result (reduced frame buffers) and the faster processing (shorter readout and
processing time).
Row reduction =
Column reduction =
Pixel binning = 00
R6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
14
15
320 x 120 320 x 60 320 x 30 Pixel field resolution: 320 x 240
2x 4x 8x Frame rate: 1x
7
8
9
10
11
12
13
14
15
7
8
9
10
11
12
13
14
15
7
8
9
10
11
12
13
14
15
00
00
00
00
01
00
00
10
00
00
11
Register 0x94:
C4
5
6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
R6 R6 R6
Figure 42: Row reduction on y-axis without binning
 160 x 120 160 x 60 160 x 30 160 x 240
4x 8x 16x 2x
7
8
9
10
11
12
13
14
15
7
8
9
10
11
12
13
14
15
7
8
9
10
11
12
13
14
15
7
8
9
10
11
12
13
14
15
C4
5
6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
R6 R6 R6 R6
01
01
00
01
01
01
01
01
10
01
01
11 Row reduction =
Column reduction =
Pixel binning =
Register 0x94:
Pixel field resolution:
Frame rate:
Figure 43: Row reduction on y-axis combined with horizontal binning
320 x 60 320 x 30 Pixel field resolution: 320 x 120
4x 8x Frame rate: 2x
7
8
9
10
11
12
13
14
15
7
8
9
10
11
12
13
14
15
7
8
9
10
11
12
13
14
15
R6 R6 R6
C4
5
6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
Row reduction =
Column reduction =
Pixel binning =
Register 0x94:
10 10 10
00 00 00
01 10 11
Figure 44: Row reduction on y-axis combined with vertical binning
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
37 / 70 Datasheet_epc660-V2.20
www.espros.com
160 x 60 160 x 30 Pixel field resolution: 160 x 120
8x 16x Frame rate: 4x
7
8
9
10
11
12
13
14
15
7
8
9
10
11
12
13
14
15
7
8
9
10
11
12
13
14
15
R6 R6 R6
C4
5
6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
Row reduction =
Column reduction =
Pixel binning =
Register 0x94:
01 01
01
01
11
11 11 11
10
Figure 45: Row reduction on y-axis combined with horizontal and vertical binning
2 x 320 x 120 2 x 320 x 60 2 x 320 x 30 Pixel field resolution:
1x 2x 4x Frame rate:
7
8
9
10
11
12
13
14
15
7
8
9
10
11
12
13
14
15
7
8
9
10
11
12
13
14
15
R6 R6 R6
C4
5
6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
Row reduction =
Column reduction =
Pixel binning =
Dual MGX mode =
Register 0x94:
1 1 1
00
00
00
00
00
00
00
01 10
Figure 46: Row reduction on y-axis combined dual phase mode
2 x 320 x 120 2 x 320 x 60 2 x 320 x 30 Pixel field resolution:
1x 2x 4x Frame rate:
7
8
9
10
11
12
13
14
15
7
8
9
10
11
12
13
14
15
7
8
9
10
11
12
13
14
15
R6 R6 R6
C4
5
6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
C4
5
6
7
8
9
10
11
12
13
Row reduction =
Column reduction =
Pixel binning =
Dual MGC mode =
Register 0x94:
1 1 1
00
00
00
00
00
00
00 01 10
Figure 47: Row reduction on y-axis combined with dual integration time mode.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
38 / 70 Datasheet_epc660-V2.20
www.espros.com
8.6. Region of interest (ROI)
The ROI allows readout and transfer the portion of the pixel-field data which is necessary for an application. The advantages are same as
for the resolution reduction: Reduced amount of data which have to be readout and processed. For integration times in the µs range, much
shorter than the row conversion time (see Figure 29), the frame rate scales with the set number of rows of the ROI.
ROI is active always and works mirrored over the top and bottom pixel-fields. The symmetric part in the bottom pixel-field is generated sim
ultaneously. Therefore, only minimum top-left [C4,R6] and the maximum bottom-right [C323,R125] coordinates in the top pixel-field need to
be set (registers 0x96 – 0x9B). The ROI starts with even row and column and ends with odd row and column. Top-left coordinates are
smaller than the bottom-right.
pixel
coordinates
ROI
dummy pixels
Top pixel field
Bottom pixel field
top-left (x1, y1)
bottom-right (x2, y2)
E EO
O ROI y max.
ROI y min.
ROI x min.
(x2', y2') O
E EO
ROI
(symmetric)
0
R6
7
8
9
0
C4
5
6
7
322
C323
327
R125
R126
127
242
243
251
R245
124
244
x
y
dummy pixels
dummy pixels
dummy pixels
(x1', y1')
320
321
ROI x max.
Figure 48: Region of interest (ROI)
The ROI registers can be changed on-the-fly via I2C all the time. The new values will be used with the next frame start. The application
must use the same ROI during the data readout.
IMPORTANT:
1. ROI can be set to a minimum rectangle of columns by rows of 6 by 2.
2. If row reduction is enabled, the minimum number of ROI rows is inversely scaled, 
e.g.: row reduction by 2 makes the minimum ROI to 6 by 4. 
3. If column reduction is enabled, the minimum number of ROI columns is inversely scaled,
 e.g.: column reduction by 2 makes the minimum ROI to 12 by 2.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
39 / 70 Datasheet_epc660-V2.20
www.espros.com
9. Imaging
9.1. Distance measurement (3D TOF)
The epc660's default modulation mode is based on the sinusoidal TOF modulation theory but uses effectively for the illumination a square
wave modulated signal with a duty cycle of 50%. After reset, all internal register values are default to operate the chip at 4MHz XTAL/ex
ternal clock input, multiplied up to 48MHz at the PLL output, clocks the modulator with 48MHz modulation clock (mod_clk), modulates
LED/LD with 12MHz and acquires 4 successive DCS frames (0 ... 3) using 47.6µs integration time.
The distance measurement mode uses the on chip LED driver and the external LED/LD to provide modulated light on the target. Modula
tion control signals to the LED driver are provided by a programmable modulator. The modulator generates all signals to modulate the ex
ternal LED/LD and simultaneously all demodulation signals to the pixel-field. TOF and grayscale mode with all the variants are generated
here.
DCS2: mga 180°, mgb 0°
mod_clk
led_mod
mga
mgb
DCS3: mga 270°, mgb 90°
mod_clk
led_mod
mga
mgb
DCS0: mga 0°, mgb 180°
mod_clk
led_mod
mga
mgb
DCS1: mga 90°, mgb 270°
mod_clk
led_mod
mga
mgb
TLED
DCS0: mga 0°, mgb 180°
mod_clk
led_mod
mga
mgb
DCS1: mga 90°, mgb 270°
mod_clk
led_mod
mga
mgb
TLED
Figure 49: 4 DCS modulation/demodulation waveforms Figure 50: 2 DCS mod./demod. waveforms
The modulation table registers 0x22 … 0x2D control the modulation (refer to Table 35). The registers can be updated via I2C bus between
frame acquisitions. The application must take care that the last frame's integration phase is completed before modifying these registers on
the fly. This time can be detected by the application by waiting for the falling-edge of VSYNC or the first falling-edge of HSYNC signal after
shutter pulse/command was applied. This allows to run continuously at the maximum frame rate. For a full-frame readout, the margin is a
3.6ms to alter these registers via I2C on the fly.
With the application of the shutter pulse (HW SHUTTER or SW shutter via I2C), the chip performs the required number of successive DCS
acquisitions. Each one of the 4 DCS frame types has a different phase relation between modulation (led_mod) and demodulation (mga,
mgb) signals which makes phase-to-distance calculation possible. In case of DCS0, led_mod is phase-shifted by 0º and 180º with respect
to mga and mgb, respectively. In case of DCS1, led_mod is phase-shifted by 90º and 270º. For DCS2, the phase shifts are 180º and 0º
and for DCS3, the phase shifts are 270º and 90º (see Figure 49). Note that for DCS2 and DCS3, the demodulation signals mga and mgb
are simply swapped with respect to DCS0 and DCS1, respectively.
By programming the number of DCS readouts = 01 (see 0x92 register), shutter initiates 2 successive DCS frame acquisitions (see Figure
50). This mode allows distance acquisition by using two DCSs only and thus a doubled frame rate. However, the cost is a lower distance
measurement accuracy and a 40% higher distance noise.
9.2. Distance calculation algorithm
The use of the trigonometric atan2 definition for vectors (x, y) in the Cartesian coordinate system φ = atan2(x, y) = atan2(y/x) guarantees a
continuous distance calculation algorithm in the range of phases between -π … +π. In our case, we use the range from 0º… 360º which
corresponds to the distance from 0m up to the unambiguity distance (refer to Figure 51 and Figure 52).
y-axis
x-axis 0
π/2-π/2-π
π
x
y x / y 
φ
tMOD
time
0
amplitude
φ
time-of-flight
APIXEL
0° 90° 180° 270° phase shift
DCS0 DCS1 DCS2 DCS3 sample
sample
emitted AC signal
received AC signal
0°
Figure 51: Continuous atan2 representation for the range -π … +π  Figure 52: Sampling of the received waveform
Typically, the distance is calculated by using the 4 DCSs, also called π-delay matching, which cancels pixels offsets leading to distance er
rors:
[2] DTOF
[m]= c
2 ⋅ 1
2πfLED
⋅
[ π+atan2 (DCS3−DCS1
DCS2−DCS0
) ] +DOFFSET
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
40 / 70 Datasheet_epc660-V2.20
www.espros.com
The measured data are always over the 360º phase-shift valid. Due to the distance offset adjustment DOFFSET, the correction of the distance
roll-over effect at zero and unambiguity distance is necessary for having all the time correct distance values D:
■ if DTOF > DUnambiguity :
■ if DTOF < 0:
D = DTOF - DUnambiguity 
D = DTOF + DUnambiguity 
■ else:
D = DTOF
If higher distance errors can be tolerated but a high frame rate is needed, the distance calculation also works with 2 DCSs only:
[3]
DTOF 
[m] = c
2 ⋅ 1
2πfLED 
⋅
[ π + atan2 (−DCS1
−DCS0
The following terms are used in the formulas above:
DTOF
c
fLED
Distance in meters [m]
Speed of light 299'792'458 [m/s]
)
]
LED/LD modulation frequency e.g. 12MHz
DCS0 - DCS3
φ
DOFFSET
DUnambiguity
Sampling amplitude [LSB]
Phase shift caused by the time-of-flight [rad]
Offset compensation [m]
Unambiguity distance
9.2.1. Unambiguity range versus time base setting
Due to continuous modulation, roll-over can be observed if the distance to the object is longer than the length of one modulation cycle (one
period, 2π). This roll-over distance is called unambiguity range can be calculated as follows:
[4]
DUnambiguity 
[m] = c
2 ⋅ 1
fLED
The operating range is the maximum distance which corresponds to the maximum time-of-flight inside of one period of the used modula
tion: It is one period of fLED. Objects inside this area are detected unambiguously.
The unambiguity range defines the repetition distance, where objects outside of the targeted operating range can still be detected as far
they are of very high reflectivity (remission). Strongly reflected signals outside of this range may therefore interfere with the measurement.
The operating range, the unambiguity distance, the time base for the integration time and the resolution of the distance signal are defined
by the modulation clock mod_clk. This corresponds for the epc660 to a maximum default operating range of 12.5m @ mod_clk = 43MHz. It
may be necessary depending on the application to adapt these parameters to other values. It can be done by a change of the modulation
clock. Table 20 lists as an example some values of the modulation clocks in function of the the unambiguity distances, of the distance res
olutions and of the multipliers of the integration time base.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
41 / 70
Datasheet_epc660-V2.20
www.espros.com
Unambiguity distance Integration time
multiplied by
[m]
          6.25
        12.5 1
   25
   50
100
[#]
  1  
  2 1
  4  
  8  
16  
Distance 
resolution 2
[cm]
0.21
0.42
0.83
1.67
3.33
Modulation clock
fMOD
[MHz]
  96
  48
  24
  12
    6
Modulation clock divider
Register 0x85
[#]
  0  
  1 1
  3  
  7  
15  
Table 20: Unambiguity range versus on-chip modulation clock
Notes:
1 Default values
2 The distance resolution is given for an operating range corresponding to 3'000 LSB.
3 Using external modulation clock MODCLK: Follow chapter 5.5.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
42 / 70
LED modulation frequency
fLED
[MHz]
24
12
  6
  3
      1.5
Datasheet_epc660-V2.20
www.espros.com
9.2.2. Quality of the measurement result
The DCS values contain not only the distance information, but also the quality and the validity (confidence level) of the received optical sig
nal. The higher the signal amplitude of the received signal, the better and more precise the distance measurement. Each distance mea
surement of every pixel has its own validity and quality.
The primary quality indicator for the measured distance data is the amplitude of the received modulated light ATOF. The amplitude is in di
rect relationship to the distance noise (refer to Figure 5). The amplitude can be calculated as follows:
[5]
ATOF 
= 
√
(DCS2−DCS0)2+ (DCS3−DCS1)2
2
Amplitude ATOF
Classification
Action
< 25 LSB
Weak illumination
Objects can be detected but distance measurement is not possible.
Increase the integration time for the next measurement.
25 … 100 LSB
Useful for measurement
High distance noise, increase the integration time
100 … 1'700 LSB
Good signal strength
No action necessary
> 1'700 LSB
Overexposed
Decrease integration time for the next measurement. 
Table 21: Signal amplitude versus classification
Note:
The amplitude value is the feedback parameter that is used to set the integration time for the next measurement. Generally, the higher the
received signal, the better and more precise the distance measurement.
9.3. Grayscale imaging
The grayscale mode allows using the epc660 as a grayscale imager. This mode can be used either without LED/LD illumination for ambi
ent-light measurements or with LED/LD for active illumination of the scenery. The grayscale measurement uses regular DCS measurement
but with DCS0 only. It is performed with differential readout using MGA only which stays on all the integration time. Data output format is
signed integer 12 bit: ± 2'047 LSB. Effective data range is 0 … +2'047. Due to system noise around zero, the readout can show small neg
ative numbers. Corresponding settings can be found in register 0x3C (= 0x26). Due to fact that distance measurement results can be influ
enced by ambient-light, the grayscale measurement without illumination can thereof be used as an important quality and correction param
eter for the distance measurement.
The saturation flag status is invalid in this mode. 
The irradiance EBW of the grayscale signal at the surface of a pixel can be calculated from the DC sensitivity SBW, the used integration time
tINT-BW, the reference integration time tINT-REF-BW and the amplitude of DCS0 of the grayscale signal as follows:
[6]
EBW 
= SBW
⋅tINT-REF-BW
tINT-BW 
⋅DCS0
9.4. Calibration and compensation of TOF cameras
e.g. 
EBW 
= 0.25nW/mm2
LSB ⋅ 100μs
1.6μs ⋅1'000 LSB = 15.6μW/mm2
This modern TOF sensor chip offers a fully digital interface to the control circuitry of a TOF camera. The first time, user naturally expects
straight forward implementation and digital accuracy of the measured signals. Unfortunately, this is often followed by tremendous disillu
sion because of the many physical effects influencing the final performance of 3D TOF cameras.
3D TOF cameras capture images by utilizing the time-of-flight measurement of photons. Photons are emitted by high frequency modulated
LEDs or laserdiodes, which are part of the camera, then scattered from objects in the scenery and finally, some of the emitted photons are
reflected back to the camera and captured in so-called demodulation pixels. This time-of-flight happens in an incredibly short period of time
as it takes place with 300'000km/s or 30cm/ns. If one would like to achieve a centimeter distance resolution and accuracy, 30ps time mea
surement accuracy has to be achieved. This is a very tough requirement, especially if tens of thousands of pixels shall provide such accu
rate measurement several dozen times per second at the same time. Small and inherent differences in the connection and arrangement of
transistors within the TOF chip, temperature differences and changes, but also irradiance signal strength and last but not least ambient
light change lead to measurement errors in the tens of centimeters:
Calibration and compensation is essential to reach the goal. To support users, ESPROS issued on the Website www.espros.com in the
section “Downloads” the application note AN10 “Calibration and compensation of Cameras using ESPROS TOF Chips”. This paper de
scribes the error sources in 3D TOF sensor chips, a simple way to implement a calibration procedure and how to compensate them on
camera level.
Other documents which can be helpful to achieve a successful implementation of the chip are listed in chapter 16.2, Related documents.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
43 / 70
Datasheet_epc660-V2.20
www.espros.com
9.5. Noise reduction and signal filtering
w/o filter
with filter
Figure 53: Effect of the static Kalman filter on distance noise (Distance in mm)
Whatever measurement process is applied, distance noise is one of the major challenging factors of 3D TOF imaging. It limits to distin
guish in depth between small objects or fine contours. It is called temporal noise and varies from measurement to measurement. Since this
noise is a statistical value, its effect can be reduced by filtering. 
However, a simple averaging with a FIR filter is not suitable in many applications because of the very long time lag to get a filtered result.
Filtering based on the theory of Rudolf E. Kalman, noise can be reduced significantly without losing responsivity of the system. Figure 53
shows the resulting effect of such a Kalman filter.
Left side: The frames 0 to 120 have been acquired without filtering at all. The distance noise is approx. 12cmpp (1 sigma = 2.5cm).
Right side: Frames 121 to 250 are processed with the Kalman filter. The distance noise is reduced to approx. 2cmpp (1 sigma = 0.5cm). 
The signal amplitude was quite low in both cases, approx. 250 LSB.
To support users, ESPROS issued on the Website www.espros.com in the section “Downloads” the application note AN12 “Distance Noise
Reduction with Kalman Filter”. This paper describes background and implementation of two Kalman filter algorithms in 3D TOF cameras.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
44 / 70
Datasheet_epc660-V2.20
www.espros.com
10. Temperature sensors
There are four temperature sensors located near the pixel-field (Figure 3). They are factory calibrated at 27°C (offset). The temperature
values can be accessed in registers 0x60 - 0x67 after taking a grayscale image. The sensitivity for taking the grayscale image with the pro
cedure described below is 2.5 times lower compared to the regular grayscale modes described in chapter 9.3. Most applications need
grayscale (or ambient-light) pictures for background-light compensation. By reading the temperature, a grayscale image can be read at the
same time.
10.1. Initialization
upon power-up or after a RESET:
define V, W, X, Y, M, # Define required variables
       array_C[4], array_Z[4], # Define required variables
       array_TH[4], array_TL[4], # Define required variables, only for temperature reading
       Temp[4] # Define required variables
V = RD @0xD3 # Save register 0xD3
W = RD @0xD5 # Save register 0xD5
X = RD @0xDA # Save register 0xDA
Y = RD @0xDC # Save register 0xDC
array_C[0] = RD @0xE8 # Read sensor top-left factory calibration
array_C[1] = RD @0xEA # Read sensor top-right factory calibration
array_C[2] = RD @0xEC # Read sensor bottom-left factory calibration
array_C[3] = RD @0xEE # Read sensor bottom-right factory calibration
# Calculate for i = 0,1,2,3
array_Z[i] = array_C[i]/4.7-0x12B # Normalized calibration values for the temperature formula
#Set defaults for grayscale
WR @0x3C = 0x26 # Ambient only (default factory setting)
WR @0x3A = 0x30 # Differential readout
Note:
The registers 0xD3, 0xD5, 0xDA, 0xDC are factory set registers (trim registers). To achieve an optimal temperature sensing, these regis
ters have to be modified before temperature reading. Afterwards, their original contents have to be restored. This procedure is described
above. If these registers are accidentally overwritten, the chip will not work anymore properly. However, the original content of these regis
ters is stored in the EEPROM. By applying a reset, the original content is restored and the chip will work as expected.
10.2. Readout during runtime
1. Set the integration time for the grayscale image the regular way. Note: The sensitivity is 2.5 times lower than in the regular grayscale
mode.
2. Acquire a grayscale image, do the temperature readout and the temperature calculation. The grayscale image will be acquired with the
following procedure and stores the temperature value into the registers 0x60 … 0x67.
M = RD @0x92 # Save mode register, control no. of DCS
WR @0xD3 = V or 0x60 # Set bits b5 and b6
WR @0xD5 = W and 0x0F # Clear bits b4 and b5
WR @0xDA = X or 0x60 # Set bits b5 and b6
WR @0xDC = Y and 0x0F # Clear bits b4 and b5
# Image acquisition
WR @0x92 = 0xC4 # Change mode to grayscale 
WR @0xA4 = 0x01 # Trigger image acquisition
# (can also be done with a hardware shutter pulse)
# Wait until the image is transferred (VSYNC goes high)
array_TH[0] = RD @0x60 # Read sensor top-left high byte
array_TL[0] = RD @0x61 # Read sensor top-left low byte
array_TH[1] = RD @0x62 # Read sensor top-right high byte
array_TL[1] = RD @0x63 # Read sensor top-right low byte
array_TH[2] = RD @0x64 # Read sensor bottom-left high byte
array_TL[2] = RD @0x65 # Read sensor bottom-left low byte
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
45 / 70 Datasheet_epc660-V2.20
www.espros.com
array_TH[3] = RD @0x66 # Read sensor bottom-right high byte
array_TL[3] = RD @0x67 # Read sensor bottom-right low byte
# Switch back to normal image acquisition
WR @0xD3 = V # Restore register 0xD3
WR @0xD5 = W # Restore register 0xD5
WR @0xDA = X # Restore register 0xDA
WR @0xDC = Y # Restore register 0xDC
WR @0x92 = M # Change back to the mode before temperature reading
10.3. Calculate temperature in °C 
#i = 0,1,2,3
Temp[i] = (array_TH[i]*0x0100+array_TL[i]-0x2000)*0.134+array_Z[i]
#Temp[0]: Sensor top-left temperature
#Temp[1]: Sensor top-right temperature
#Temp[2]: Sensor bottom-left temperature
#Temp[3]: Sensor bottom-right temperature
Note:
The grayscale image which has been acquired can be used. However, the sensitivity during this acquisition was reduced by a factor of 2.5.
Thus, if the same sensitivity should be needed, the integration time has to be increased with a multiplier of 2.5.
In order to reduce temporal noise on the temperature measurement, the following filtering algorithm is recommended.
1. Spatial averaging over the 4 temperature sensors.
2. Temporal filtering with a Kalman filter.
x[i] = (Temp[0]+Temp[1]+Temp[2]+Temp[3])/4 # Spatial averaging
k = 0.1 # Kalman gain
y[i-1] = x[0] # Start condition
y[i]=k*x[i]+(1–k)*y[i-1] # Simple Kalman filter
x[i]: Current spatial averaged temperature
y[i]: Current temporal filtered temperature
y[i-1]: Previous temporal filtered temperature
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
46 / 70 Datasheet_epc660-V2.20
www.espros.com
11. Application information
11.1. Start-up and initialization sequence
11.1.1. Default
1. RESET = 0.
2. Apply all supplies (chapter 5.6.1).
3. RESET = 1.
4. Continue when copying from EEPROM to CFG is finished.
5. Write pixel sequencer code to memory (chapter 15.11)
6. Enable LED preheat (chapter 1.6).
7. Set registers as shown in 22:
Address
Set to
Comments
0x7E
bit 0 = 1
Read this register for a feedback if EEPROM to CFG copied
0x90
bit 3 = 1
0xAB
0x01
Enable LED Preheat
0xAE
11.1.2. Customer specific
0x04
■ Set modulation clock to external.
Enable manual DLL control
Table 22: Additional register settings during startup 
■ Set custom I2C slave address with strap pins (chapter 5.6.3).
■ Set TCMI mode and polarity.
■ Set integrated LED driver according to used illumination.
■ The registers as shown in 23:
Address
Comments
0x80
Enable internal clk and external modulation clock. Set therefore address 0x80 to 0x7F.
0xCB
I2C and TCMI control
0xCC
TCMI polarity settings
0x90 
LED/LED2 driver control
11.2. Image acquisition
11.2.1. 3D TOF mode
■ Select acquisition mode:
■ Set registers as shown in 24:
Table 23: Customer specific register 
Address
Set to
Comments
0x92
0x34
4 DCS TOF mode
0x92
11.2.2. Grayscale mode
■ Enable Grayscale mode
■ Differential readout with ABS
0x1C
2 DCS TOF mode
Table 24: Register settings for DCS mode 
■ Adjust the saturation threshold to get a better image. Important: Set it back for taking a 3D TOF image.
■ Switch to temperature sensing mode
■ Set registers as shown in 25:
Address
Set to
Comments
0x92
0xC4
Change mode to grayscale
0x3A
0x30
Select readout mode to ABS
0xAF
0x39
Saturation threshold for 125us integration time
0xD3 - 0xDC
Temperature sensing mode according to chapter 10.2
Table 25: Additional register settings during grayscale mode 
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
47 / 70
Datasheet_epc660-V2.20
www.espros.com
11.2.3. Dual phase mode selection (motion blur reduction)
Refer for the description to chapter 8.2.
■ This mode needs the following basic setting of the register 0x94 = 0x80, register 0x22 = 0x34 and register 0x25 = 0x3E.
■ Reset the registers to the default values after leaving this mode: register 0x94 = 0x00, register 0x22 = 0x30 and register 0x25 = 0x35.
Function
Register 0x92
Comments
4x DCS  
not applicable
2x DCS 2
0x14
Output is effectively 4x DCS in 2 DCS-frames.
Grayscale
not applicable
Table 26: Setting basic dual phase mode
11.2.4. Dual integration time mode selection (high dynamic range)
Refer for the description to chapter 8.3.
■ This mode needs the following basic setting of the register 0x94 = 0x80.
■ Reset the register to the default value after leaving this mode: register 0x94 = 0x00.
■ Output is 2 equal DCS frames with different integration times in one readout frame.
Mode
Register setting
Function
Comments
Register 0x92
Register 0x3C
4x DCS
0x3C
0x26
2x DCS
0x1C
0x26
Ambient only
0xCC
0x26
Grayscale imaging, no active illumination
Ambient & non modulated LED/LD
0xCC
0x16
Grayscale with DC illumination
Ambient & modulated LED/LD
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
0xCC
0x06
Grayscale with modulated illumination
Table 27: Setting dual integration time mode for TOF and grayscale
48 / 70
Datasheet_epc660-V2.20
www.espros.com
11.3. Configuration sequence
This example shows a normal image acquisition with four DCS and one grayscale image. 
DCS0
t = intTOF
dataout
tDataout=6.3ms
DCS 1
t = intTOF
dataout
tDataout=6.3ms
DCS 2
t = intTOF
dataout
tDataout=6.3ms
DCS 3
t = intTOF
dataout
tDataout=6.3ms
time line
I2C
epc660
state VSYNC
Config3
integration
grayscale
t = intgrayscale
dataout
Grayscale
tDataout=6.3ms
Config4
+ 
read
temp.
t_shutter_lag = 18µs
next 
DCS
Config
power up
Config1 Config2
single shutter
single shutter
single shutter
t_shutter_lag = 18µs
t_shutter_lag =18µs
Figure 54: Sequence for normal DCS mode
Table 28: I2C command with description and required time
Action I2C commands Comment / description required time 
I2C command
SCL = 1MHz
Config
power up
WR 0x3A = 0x30
WR 0x3C = 0x26
WR 0x85 = 0x01
WR 0xA0, 0xA1, 0xA2, 0xA3 = 0x..
WR 0x92 = 0x34
has to be initialized only once after system power up
has to be initialized only once after system power up
set modulation frequency with MOD_CLK_divider
configure integration time = intTOF
Configure 4 DCS mode
29µs (1x I2C write)
29µs (1x I2C write)
29µs (1x I2C write)
4 x 29µs = 116µs (4x I2C w.)
29µs (1x I2C write)
Single shutterWR 0xA4 = 0x01 a) trigger SW shutter  
b) trigger HW shutter (faster than SW trigger) 
29µs (1x I2C write)
HW trigger lag = 3us
Config 1 WR 0x92 = 0xC4
WR 0xA0, 0xA1, 0xA2, 0xA3 = 0x..
configure grayscale mode
configure integration time = intgrayscale
29µs (1x I2C write)
4 x 29µs = 116µs (4x I2C w.)
Config 2 WR 0xD3, 0xD5, 0xDA, 0xDC = 0x.. Modify register values according data sheet (normal 
sensing mode)
4 x 29µs = 116µs (4x I2C w)
Single shutterWR 0xA4 = 0x01 a) trigger SW shutter  
b) trigger HW shutter (faster than SW trigger) 
29µs (1x I2C write)
HW trigger lag = 3us
Config 3 WR 0xA0, 0xA1, 0xA2, 0xA3 = 0x..
WR 0x92 = 0x34
configure integration time = intTOF
Configure 4 DCS mode
4 x 29µs = 116µs (4x I2C w.)
29µs (1x I2C write)
Config 4
+ 
read temp.
WR 0xD3, 0xD5, 0xDA, 0xDC = 0x..
RD 0x60, 0x61, … , 0x67
Modify register values according data sheet (temper
ature sensing mode)
get values for temperature calculation
4 x 29µs = 116µs (4x I2C w.)
8 x 39µs = 312µs (4x I2C r.)
Single shutterWR 0xA4 = 0x01 a) trigger SW shutter  
b) trigger HW shutter (faster than SW trigger) 
29µs (1x I2C write)
HW trigger lag = 3us
NOTE: Config registers can be updated on-the-fly while a frame acquisition is going on. The new values are used at the start of the next frame.
11.4. Integration time setting
The integration time is the active frame acquisition period (see Figure Figure 29). Specially for moving objects or cameras, this time should
be as short as possible to reduce or eliminate motion blur effects. The integration time together with the illumination intensity also defines
the effective achievable operating distance. Using the on-chip modulation clock, the integration time can be calculated as
[7] tINT
= reg(0x85)+1
96MHz ⋅[reg(0xA2:0xA3)+1]⋅reg(0xA0:0xA1)
Table Table 29 lists some useful integration time settings.
Integration time Registers (0xA0:0xA1) Registers (0xA2:0xA3)
[DEC] [HEX] [DEC] [HEX]
1.58 μs 1d 0x0001       75d 0x004B
12.5 μs 1d 0x0001     599d 0x0257
 100 μs 1d 0x0001  4'799d 0x12BF
 800 μs 1d 0x0001 38'399d 0x95FF
  1.6 ms 2d 0x0002 38'399d 0x95FF
Table 29: Typical TOF and grayscale integration times for 12MHz on-chip modulation frequency (modulation clock = 48MHz)
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
49 / 70 Datasheet_epc660-V2.20
www.espros.com
11.5. Power consumption
The epc660 has several power states/levels during the different operation phases which are shown in Table 30 and Figure 55.
Power state Power [mW] Operation description
RESET     54 All supplies are ON, RESET = 0,
Oscillator is ON, PLL and all system system clocks are OFF
READY   110 RESET = 1, PLL and all system clocks ON, waiting for SHUTTER
INTEGRATION 1'300 SHUTTER pulse/command
CONVERSION   555 Integration finished, conversion of rows
CONVERSION + DATAOUT   580 Transmit row data via TCMI while converting next row
DATAOUT   110 Transmit last row data via TCMI
Table 30: Typical average power consumption levels at different operating states (integration time < 5ms)
Con...
Dat...
Frames Conversion even
R125, R126
Data out
R125
Conversion odd
R125, R126
Data out
R126
Conversion even
R124, R127
Conversion odd
R124, R127
Data out
R124
Data out
R127
Integration Init
Conversion odd
R6, R245
Integration Init Con...
SHUTTER
RESET
INTEGRATION READY RESET CONVERSION CONVERSION CONVERSION CONVERSION DATAOUT
DATAOUT
Power
consumption
Figure 55: Power consumption levels and operating states
For power critical applications e.g. battery powered systems, it is possible to enforce the epc660 to go in so-called power saving states.
No. Register Description
Name Address Value
Power down
1 Power control 0xA5 0x00 Switch off of unnecessary supplies
2 Clock control 0x80 0x00 Switch off of unnecessary clocks
3 Mode control 0x7D 0x14 Switch system clock to XTAL clock
4 Mode control 0x7D 0x10 Switch off PLL
Power up
5 Mode control 0x7D 0x14 Switch on PLL
5 Wait > 32μs Wait until PLL stable
7 Mode control 0x7D 0x04 Switch system clock to PLL
8 Clock control 0x80 0x3F Switch on the clocks again
9 Power control 0xA5 0x07 Switch on the supplies again
10 Wait until supplies are stable
11 Regular 3D TOF operation
Table 31: Sequence for the SW POWER DOWN mode
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
50 / 70 Datasheet_epc660-V2.20
www.espros.com
11.6. Rolling DCS frames
In special applications, it is possible to use all the time the same integration time in continuous distance measurement mode without any
grayscale images for ambient-light compensation. Such a set-up allows enhancing the distance measurement rate by a factor of 4 by us
ing rolling DCS frames.
Frames
...
DCS3
1st sequential distance image 
DCS1
DCS2
DCS3
2nd sequential distance image 
DCS0 DCS0
DCS0
1st rolling distance image 
2nd rolling distance image 
DCS1
3rd rolling distance image 
DCS2
4th rolling distance image 
DCS3
5th rolling distance image 
Figure 56: Rolling DCS frames
...
As shown in Figure 56, the algorithm performs with each new DCS frame a new distance calculation based on the new and last three DCS
frames.
11.7. Enhanced rolling DCS frame mode
epc660 allows to set for each single DCS access own parameters. This opens also the possibility to acquire in time-sequence DCSx
frames with e.g. different integration times.
The enhanced rolling mode combines all:
The stacking of integration times to enlarge the dynamic range, the acquisition of an ambient-light image for correction and the rolling
mode to speed up the frame rate.
The final distance frame acquisition will be in an equidistant time manner e.g. for 2 or more different integration times.
Select out of the acquired integration time distance frames, already compensated, each time the most reliable distance information for the
final composed distance picture
The example shown here is using two integration times: 
50µs for detecting short range objects and 2ms doing the same for the long range.
DCS Acquisition
Illumination
ADC & Readout
Data transfer epc660
DCS0(n)
ADC
ADC
DCS1(n)
ADC
TCMI
TCMI
TCMI
DCS2(n)
ADC
ADC
ADC
TCMI
TCMI
DCS3(n)
ADC
ADC
DCS0(n+1)
short & long distance
50µs integration time
short distance 0.15 … 0.8m
TCMI
FPGA Processing
Data transfer FPGA
TOF frame 0.15 … 5.0m
16b Parallel
16b Parallel
Distance frame (x-1)
16b Parallel
TOF image (x)
Distance frame (x)
TCMI
TCMI
16b Parallel
Distance frame (x+1)
16b Parallel
Distance frame (x+2)
10ms equiv. 100fps
Figure 57: Enhanced rolling mode sequence
Implementation example step by step: Rolling mode using 3 integration times
1. Chose single frame mode by setting register 0x22 and 0x92.
2. Run 4 DCS turns by
3. Select DCS0 and acquire 3 DCS0 each with one of the 3 integration times
2ms integration time
long distance 0.8 ... 5.0m
Integration time t1 > shutter > readout > integration time t2 > shutter > readout > integration time t3 > shutter > readout.
2nd and following turns:
Calculate for each integration time the distance and TOF amplitude image with the last 4 corresponding DCS frames.
Select out of the acquired integration time distance images, already compensated, each time the most reliable distance information 
and compose the actual final distance picture.
4. Select DCS1 and acquire 3 DCS1each with one of the 3 integration times
Integration time t1 > shutter > readout > integration time t2 > shutter > readout > integration time t3 > shutter > readout
2nd and following turns:
Calculate for each integration time the distance and TOF amplitude image with the last 4 corresponding DCS frames.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
51 / 70
Datasheet_epc660-V2.20
www.espros.com
Select out of the acquired integration time distance images, already compensated, each time the most reliable distance information 
and compose the actual final distance picture.
… and so on ...
Register
0x22
0x25
0x92
Mode
DCS/Shutter
DCS select
1st frame
DCS select
2nd frame
Modulation
select
4 DCS
DCS 0, 1, 2, 3
0x34
0x3D
0x30
2 DCS
DCS 0, 1
0x34
0x3D
DCS 2, 3
0x32
0x33
0x10
1 DCS rolling
DCS 0
0x34
DCS 1
0x31
DCS 2
0x32
DCS 3
0x33
Not used
Table 32: DCS selection for enhanced rolling mode
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
52 / 70
0x00
Datasheet_epc660-V2.20
www.espros.com
12. Parameter and configuration memory
0x00 IC TYPE
IC VERSION 0x01
0x7F
Control Page
R/W (128 x 8-bit)
0x11 EE ADDR
Data access:
only with
indirect EEPROM 
access
0x80
0xFF
0xF0
RAM Page
R/W (128 x 8-bit)
0x80
EEPROM Page
EEPROM (128 x 8-bit)
0xFF
reserved
0xFB PART VERSION
PART TYPE
CHIP ID LSB
CHIP ID MSB
0xFA
0xF9
0xF8
WAFER ID LSB
WAFER ID MSB
0xF7
0xF6
0xF0
Copy after reset
Indirect access through EEPROM page and access registers
Operating 
Memory
R/W
0x00
0x7F
0x80
0xF0
0xFF
I2C
Figure 58: Memory map
12.1. Data memory map
The epc660 control registers (RAM) are used for controlling all features of the chip. They are organized as 256x8 bit into 0x00 ... 0xFF ad
dress locations. The address space 0x80 ... 0xFF is EEPROM backed-up. EEPROM parameters in this section are stored permanently be
tween the power off/on cycles. All registers can be accessed through I2C interface by the application CPU (see chapter 13, I2C interface).
Multiple byte registers are stored in the order MSB first, then LSB.
12.1.1. Control page
The control page contains R/W accessible registers with default values during startup. The content can be changed via the I2C interface.
The changed values are preserved as long as the IC is powered. They are set back to their default values with a reset.
12.1.2. RAM page
The RAM page contains R/W accessible registers with EEPROM copied values after startup. The content can be changed via the I2C inter
face. The changed values are preserved as long as the IC is powered. They are set back to EEPROM values with a reset.
12.1.3. EEPROM page
The embedded 128x8-bit EEPROM stores operation parameters as well as factory set trimming and calibration values.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
53 / 70 Datasheet_epc660-V2.20
www.espros.com
13. I2C interface
The I2C-bus interface allows accessing the RW registers and the programming of the EEPROM registers which store the configuration pa
rameters. It is the only interface through which the configuration registers can be accessed (Figure 58 and Table 35) by the application. It
works as a slave device according to the I2C specification (refer to chapter 16.2) with a transfer rate of up to 400 kbit/s in Fast Mode (FM)
or 1Mbit/s in Fast Mode plus (FM+). The I2C master such as an external CPU can set the transfer speed simply by driving the SCL input at
that frequency (up to 1MHz), therefore there is no prior register configuration or setting necessary.
I2C specification is supported in epc660 with following remarks/exceptions:
■ 7-bit addressing only is supported.
■ Clock stretching is supported.
■ General call address: By transmitting 0x00 followed by 0x06 (issues software reset) or transmitting 0x00 followed by 0x04 (device ad
dress reload), the programmable part (A0, A1) of the I2C address pins is overwritten by the initially scanned value through strap pins dur
ing start-up or reset phase.
■ Software reset is supported. 
■ Other uses of I2C bus are not supported.
13.1. Device addressing
The epc660 7-bit I2C device address is hard-wired to the value shown below in Figure 59. Two address bits A0, A1 can be optionally initial
ized as 1 through strap pins (chapter 5.6.3). In a typical single-camera 3D TOF imager application in which epc660 is directly connected as
a single I2C slave to a single I2C master, the strap pins can be can be left open. An internal pull down resistor keeps them low. In this case,
the device address is set after reset default as 0100000. In a multi-camera application with up to 4 epc660 devices connected on the same
I2C bus as slaves or together with other I2C slaves talking to a single I2C master, external pull-up resistors can be utilized on the strap pins
to initialize different I2C device addresses in order to correctly identify different epc660 slaves on the bus.
Figure 59: Device address through I2C
13.2. I2C bus protocol notation
The following notation is used:
■ S
■ P
■ A
■ A
START condition
STOP condition
Acknowledge last byte (ACK)
Not-Acknowledge last byte (NACK)
■ Shaded part of protocol: transmitted by master
■ Unshaded part of protocol: transmitted by epc660
13.3. I2C bus timing
SDA
START condition
MSB LSB
STOP condition
ACK
SCLK
START condition
Data stable
tSDAF
SDA
SCL
tSTA
Acknowledgment
Change of data allowed
tSDAR
MSB LSB
tSCLH
tSU
tH
tSTOSTA
ACK
tSTO
tSCLF
tSCLR
tSCLL
Figure 60: I2C bus timing
Symbol
Parameter
Min.
Max.
Units
tSCLL
SCL clock low time
0.5
µs
tSCLH
SCL clock high time
0.26
µs
tSU
SDA setup time
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
50
54 / 70
ns
Datasheet_epc660-V2.20
www.espros.com
tH
SDA hold time
0
ns
tSDAR / tSCLR
SDA and SCL rise time
120
ns
tSDAF / tSCLF
SDA and SCL fall time
120
ns
tSTA
Start condition hold time
0.26
µs
tSTO
Stop condition setup time
0.26
µs
tSTOSTA
Stop to start condition time (bus free)
0.5
µs
Cb
Capacitive load for each bus line
550
pF
tSP
Pulse width of the spikes that are suppressed by the analog filter
Table 33: I2C bus timing: Timing parameters (FM+)
13.4. I2C commands
13.4.1. Software reset
(0x00, 0x06) issues a software reset, same behavior like hardware reset.
13.4.2. Device address reload
Figure 61: Software reset through I2C
50
ns
(0x00, 0x04) activates the I2C address stored in register 0xCA. Note that the the values of A0 and A1 cannot be changed by software.
Therefore, this general call command only works for bits 2 to 6 of register 0xCA (chapter 5.6.3).
Figure 62: Device address A1, A0 reload through I2C
13.4.3. Write single-byte
During a single-byte write, only one register is written. After the device address is transmitted, the master has to transmit the register ad
dress and the write data in two I2C data packets (Figure 63). The access is terminated by a STOP condition.
Figure 63: Single-byte Write access through I2C
13.4.4. Write multi-byte
During a multi-byte write operation, the master transmits the device address and the address of the first register to be written. All subse
quent bytes until the STOP condition are interpreted as write data packets (Figure 64). The write address pointer is incremented internally.
Do not transmit more bytes that the write address pointer reaches the limit of the address space (see chapter 14, Table 35 andTable 36).
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
Figure 64: Multi-byte Write access through I2C
55 / 70
Datasheet_epc660-V2.20
www.espros.com
13.4.5. Read single-byte
The master transmits first the device address with a write command. Next, it writes the register address to be read. Then, the master trans
mits the device address again with a read command where the epc660 answers with the data stored in the addressed register. Finally, the
master terminates the read sequence with a NACK and a STOP (Figure 65).
13.4.6. Read multi-byte
Figure 65: Single-byte Read access through I2C
The master transmits first the device address and the address of the first register to be read. After the epc660 is addressed with a read
command, epc660 answers with read data bytes until the master does not acknowledge a byte. The master is expected to terminate the
access with a STOP condition thereafter (Figure 66). During the access the read address pointer is incremented epc660 internally. Do not
transmit more bytes that the write address pointer reaches the limit of the address space  (see chapter 14, Table 35 andTable 36).
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
Figure 66: Multi-byte Read access through I2C
56 / 70
Datasheet_epc660-V2.20
www.espros.com
13.5. Command timing
The operating modes of the epc660 are initialized, activated, deactivated and monitored by sending several single or multi-byte write and
read command sequences through I2C interface. This section lists and explains available commands together with their access time (fSCL =
1MHz → tSCL = 1µs).
There is no particular order defined for sending the commands. The only requirement is having no on-going frame acquisition process
when updating non-shadowed registers. The registers marked with ** in the register map can be updated on-the-fly during a frame acquisi
tion. New values are used by the next frame.
Command
Description
Length
[Bytes]
Time
[µs]
Single-byte Write
Single-byte write to control registers
3
29
Multiple-byte Write
Multiple-byte write (n bytes) to control registers
2 + n
20 + n x 9
Single-byte Read
Single-byte read from control registers
4
39
Multiple-byte Read
Multiple-byte read (n bytes) from control registers
3 + n
30 + n x 9
Mode set
4, 2, or 1 DCS set using register 0x92
3
29
Integration time (short) set
Integration time set (up to 800µs) using integration length 1 register 
4
38
Integration time (long) set
Integration time set using integration time multiplier and length 1 registers
6
56
Dual Integration time (long) set 
Dual int. time set using integration time multiplier and length 1, 2 registers
8
74
Binning, resolution reduction set
Binning and row reduction set using register 0x94
3
29
ROI set
Region of interest set using registers 0x96 – 0x9B.
8
74
Shutter
Start frame acquisition by using the shutter control register
3
29
Integration time (short) + Shutter
Integration time + soft shutter in one go! (Integration length 1 registers, 
shutter control register)
5
47
EEPROM Indirect Single Write
Indirect single write to EEPROM
9
20ms
EEPROM Indirect Single Read
14. Register map
Notes:
**
Indirect single read from EEPROM
Table 34: I2C Control commands summary
10
97
Shadow registers can be updated on-the-fly while a frame acquisition is going on. The new values are used at the start of the 
next frame.
Not listed registers are reserved and must not be altered by the user. Otherwise, chip malfunction can occur. However, if a regis
ter is accidentally overwritten, a RESET restores the factory settings.
The listed default values are after downloading the latest sequencer program to the chip.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
57 / 70
Datasheet_epc660-V2.20
www.espros.com
14.1. Control page 0x00 ~ 0x7F
Addr. Type Default Description
0x00 R---IC type for device family identification. For chip type refer to register 0xFA.
0x01 R---IC version for device mask identification. For chip version refer to register 0xFB.
0x11 R/W---Address register for indirect read/write access to EEPROM (refer to 15.6 and 15.7)
0x12 R/W---Data register for indirect read/write access to EEPROM (refer to 15.6 and 15.7)
0x20 R 0x00 Strap scan register. Refer to 5.6.3.
Bit Function Default
0..4 reserved 0
5 Strap input 0: I2C address A0 0
6 Strap input 1: I2C address A1 0
7 reserved 0
Default start-up values of these registers are only valid until end of reset phase. Values might be over
written by external pull-up resistors during strap scan phase when reset is released.
0x22 R/W 0x30 DCS and ABS selection for 1st frame. Refer to chapter 8
Bit Function Default
0 DCS number for mgx0 modulator (mga0, mgb0), all modes
00:  DCS 0 
01:  DCS 1
10:  DCS 2
11:  DCS 3
0
1 0
2 DCS number for mgx1 modulator (mga1, mgb1), dual modes only
00:  DCS 0 
01:  DCS 1
10:  DCS 2
11:  DCS 3
0
3 0
4 Extended background suppression ABS. Refer to chapter 1.6 and 7.2.
00:  ABS disabled, Saturation detection not active
01:  reserved
10:  reserved
11:  ABS enabled (default). Refer to Table 6
1
5 1
6, 7 reserved 0
0x24 R/W 0x00 Modulation control 1st frame. Refer to chapter 8
Bit Function Default
0..3 reserved 0
0:  LED/LD is modulated
1:  LED/LD on during integration: Refer to IMPORTANT NOTE chapter 5.7
0
0
0:  LED/LD is modulated
1:  LED/LD off during integration
0
0
6, 7 reserved 0
0x25 R/W 0x35 DCS and ABS selection for 2nd frame. Description see register 0x22.
0x27 R/W 0x00 Modulation control 2nd frame. Description see register 0x24.
0x28 R/W 0x3A DCS and ABS selection for 3rd frame. Description see register 0x22.
0x2A R/W 0x00 Modulation control 3rd  frame. Description see register 0x24.
0x2B R/W 0x3F DCS and ABS selection for 4th frame. Description see register 0x22.
0x2D R/W 0x00 Modulation control 4th frame. Description see register 0x24.
0x3A R/W 0x10 Readout mode for grayscale
0x00: differential readout. Select this mode by the user application, refer to chapter 9.3 and 10.1
0x10: single-ended readout (negative numbers are clipped)
0x30: differential readout with ABS (recommended)
0x3C R/W 0x26 Modulation control in grayscale mode. Refer to chapter 9.3 and Table Table 27.
Bit Function Default
0 reserved 0
1..2 reserved 1
3 reserved 0
4 0: LED/LD modulated
1: LED/LD on during integration
0
5 0: LED/LD modulated
1: LED/LD off during integration
1
6..7 reserved 0
Table 35: Address map of the control page (0x00 ~ 0x7F)
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
58 / 70 Datasheet_epc660-V2.20
www.espros.com
Addr. Type Default Description
0x60 R---Temperature sensor top left, refer to chapter 10.
Sum of 4 consecutive readings of the temperature sensor every 4th row reading 0x61 R--
0x62 R---Temperature sensor top right.
Description see register 0x60. 0x63 R--
0x64 R---Temperature sensor bottom left.
Description see register 0x60. 0x65 R--
0x66 R---Temperature sensor bottom right
Description see register 0x60. 0x67 R--
0x71 R/W 0x00 Number of fine DLL delay steps to delay the LED output by approx. 10ps per step. Max. value is 799 
(0x31F). Valid only if bit 2 in register 0xAE is enabled. Refer also to register 0xAE and chapter 5.8. 
Note: Delay is sensitive to VDD variations and noise. 0x72 R/W 0x00
0x73 R/W 0x00 Number of coarse DLL delay steps to delay the LED output by approx. 2ns per step. Max. value is 49 
(0x31). Valid only if bit 2 in register 0xAE is enabled. Refer also to register 0xAE and chapter 5.8. 
Note: Delay is sensitive to VDD variations and noise.
0x7D R/W 0x04 Mode control
Bit Function Default
0..1 reserved 0
2 Enable PLL
0:  disable
1:  enable
1
3..7 reserved 0
Cont. Table 35: Address map of the control page (0x00 ~ 0x7F)
14.2. RAM page (0x80 ~ 0xEF)
Addr. Type Default Description
0x80 R/W 0x3F Clock control
Bit Function Default
0..5 reserved 1
6 Modulation clock source
0: Internal modulation clock
1: External clock from MODCLK input
0
7 reserved 0
0x85 R/W 0x01 Modulation clock divider
Bit Function Default
0 Modulation clock divider provides clock to the LED/Pixel-field modulator/demodula
tor circuits by integer division of the internal PLL clock or external MODCLK:
fmod_clk = 96MHz / (modulation clock divider + 1)
Default: 96MHz / (0x01 + 0x01): fmod_clk = 48MHz
Maximal value of modulation clock divider = 0x1F: fmod_clk =  3.0MHz
Note: The LED modulation frequency is 4 times lower than fmod_clk
1
1 0
2 0
3 0
4 0
5..7 reserved 0
Table 36: Address map of RAM page (0x80 ~ 0xEF)
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
59 / 70 Datasheet_epc660-V2.20
www.espros.com
Addr. Type Default Description
0x89 R/W 0x03 TCMI clock control
Bit Function Default
0 TCMI clock divider:
ftcmi_clk = 96MHz / (TCMI clock divider + 1)
Default: 96MHz / (0x03 + 0x01) = 24MHz
Minimal value of TCMI clock divider =  0x01 = 48.0MHz
Maximal value of TCMI clock divider = 0x1F =   3.0MHz
Important: Regarding readout rollover, refer to Figure 30 and register 0x91 regard
ing DCLK stretch
1
1 1
2 0
3 0
4 0
5..6 reserved 0
7 DCLK skew enable:
0:  disable 
1:  enable
Used to delay DCLK edge (typ. 2ns) to compensate PCB delays.
Might be particularly useful when TCMI clock divider = 0 (divided by 1). 
When set normal, DCLK edge is centred with respect to other TCMI *SYNC*, 
DATA[11:0] outputs. 
0
0x8B R/W 0x01 Number of PLL clock periods delay of the demodulation signal path (all modulation modes). It can be 
used to insert a phase shift between modulation (LED) and demodulation (pixel). 1 PLL clock cycle is 
around 10.4ns @ 96MHz PLL clock. This is equivalent to a distance shift of 1.5625m independent of the
LED modulation frequency. Note: This phase shift is temperature independent.
  0:  no delay
  1:  1 clock
  2:  2 clocks
  …
12:  12 clocks (max. value)
0x90 R/W 0xC4 LED driver control. Refer to chapter 5.3 and 5.7.
Bit Function Default
0 reserved 0
1 Inverts output signals LED and LED2 if drivers are enabled
0:  not inverted, e.g. LED = 0, not active: Pin LED non-conductive, LED2 = VSSIO.
1:  inverted, e.g. LED = 0, not active: Pin LED conductive, LED2 = VDDIO.
0
2 LED output select:
0:  LED driver is disable. Pin LED is non-conductive.
1:  LED driver is enabled.
1
3 LED preheat enable
0:  disable
1:  enable
0
4 LED/LD permanently on (torch function, no modulation) if drivers are enabled:
0:  off
1:  on (Refer to IMPORTANT NOTE chapter 5.7)
0
5 LED2 output select:
0:  LED2 driver disabled. Output is in Tri-State with termination resistor to VSSIO.
1:  LED2 driver enabled.
0
6..7 reserved 1
0x91 R/W 0x03 Sequencer control
Bit Function Default
0..1 reserved 1
3..5 reserved 0
6 Avoids readout rollover when using slower DCLK < 13MHz and default ROI. 
Stretches HSYNC for slower TCMI interface. Causes reduced DCS frame rate due 
to additional 2µs per ADC conversion (tconv + 2µs). Refer also to register 0x89.
0:  disable (default)
1:  enable for DCLK < 13MHz and default ROI. Refer to Figure 30 and note above.
0
7 reserved 0
Cont. Table 36: Address map of RAM page (0x80 ~ 0xEF)
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
60 / 70 Datasheet_epc660-V2.20
www.espros.com
Addr. Type Default Description
0x92** R/W 0x34 Modulation select
Bit Function Default
0..1 reserved 0
2 reserved 1
3 Dual integration time mode – acquisition with 2 integration times per DCS frame 
using additionally integration length 2, registers 0x9E and 0x9F
0:  disable
1:  enable
Needs register 0x94 set to 0x80, otherwise it is not effective (see Figure 43)
0
4 Number of DCS readouts select:
00:  Grayscale mode, DCS0 only
01:  Dual phase mode, DCS0, DCS1 or DCS2,DCS3
10:  reserved
11:  Full resolution mode or dual int. mode, DCS0, DCS1, DCS2, DCS3
1
5 1
6 Modulation select:
00:  TOF mode
01:  reserved
10:  reserved
11:  Grayscale mode
0
7 0
0x94** R/W 0x00 Resolution reduction, binning and pixel-field mode
Bit Function Default
0 Column reduction: resolution on x-axis. Refer to chapter 8.5.
00:  no (0, 1, 2, …)
01:  by half (0, 2, 4, …)
10 & 11:  reserved
0
1 0
2 Row reduction: resolution on y-axis. Refer to chapter 8.5
00:  no (0, 1,   2, …)
01:  by half (0, 2,   4, …)
10:  a quarter (0, 4,   8, …)
11:  one eight (0, 8, 16, …)
0
3 0
4 Pixel binning. Refer to chapter 8.4.
00:  no binning
01:  binning x-axis if bit 0, 1 <> 00
10:  binning y-axis if bit 2, 3 <> 00
11:  binning x and y-axis if bit 0, 1 <> 00 AND bit 2, 3  <> 00
Notes:- Choose corresponding row and/or column reduction to binning selection.- Binning cannot be used with dual phase and dual integration time mode.
0
5 0
6 reserved 0
7 Select pixel-field mode (refer to chapter 8.1, 8.2, 8.3)
0:  Standard TOF mode: full resolution 
1:  Dual modes: dual phase and dual integration time
0
0x96** R/W 0x00 ROI top left X setting. Refer to chapter 8.6. 0x97** R/W 0x04
0x98** R/W 0x01 ROI bottom right X setting. 0x99** R/W 0x43
0x9A** R/W 0x06 ROI top left Y setting.
0x9B** R/W 0x7D ROI bottom right Y setting.
0x9E** R/W 0x07 Integration length 2: Number of modulation clock periods for the second integration time in the dual inte
gration time mode (refer to 8.3, default: 2'047). See registers 0xA2 and 0xA3 for functional definition de
tails.
Bit 3 in register 0x92 has to be set to 1 to enable this integration time for the even rows. The odd rows 
operate with the integration length 1 set in registers 0xA2 and 0xA3.
0x9F** R/W 0xFF
0xA0** R/W 0x00 Integration time multiplier (10 bit value) for integration lengths set with the integration length registers 
(default = 1, min. value = 1). This multiplier is active on both settings integration length 1 and 2. 0xA1** R/W 0x01
0xA2** R/W 0x07 Integration length 1: Number of modulation clock periods for the (first in dual integration time mode ) in
tegration time (16 bit value, default = 2'047, min. value = 7 which is integration time 167ns @ 12MHz).
Integration time  = Integration time multiplier * (Integration length +1) * tmod_clk 
e.g. for defaults @ 12MHz modulation clock = 42.6µs
Note: (Integration length + 1) should be evenly divisible by 4.
0xA3** R/W 0xFF
Cont. Table 36: Address map of RAM page (0x80 ~ 0xEF)
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
61 / 70 Datasheet_epc660-V2.20
www.espros.com
Addr. Type Default Description
0xA4 R/W 0x00 Shutter Control
Bit Function Default
0 Shutter release. Refer to chapter 6.2.
0:  disable
1:  enable. In single shot mode: Starts acquisition and is auto cleared.
Note: Shutter release is not auto-cleared when multiple frames is enable.
0
1 Multiple frames (auto-run or video mode). Refer to chapter 6.2.
0:  disable. Single shot mode.
1:  enable. Multiple frame mode active if shutter enabled. Refer to chapter 6.2.2.
0
2..7 reserved 0
0xA5 R/W 0x07 Power control. Refer to chapter 11.5.
0x00: Power off
0x07: Power on
0xAB R/W 0x04 Only valid configurations (in combination with register 0x90):
LED preheat enabled: 0x90, bit 3 = 1 and 0xAB = 0x01 (recommended configuration)
LED preheat disabled: 0x90, bit 3 = 0 and 0xAB = 0x00 
0xAE R/W 0x01 DLL control (Refer also to register 0x73 and chapter 5.8)
0x01: no delay
0x04: delay manually set by register 0x73
Note: The change of register 0xAE from 0x01 to 0x04 generates also a delay, even if register 0x73 is 
set to 0x00.
0xAF R/W 0x0D Saturation threshold (factory setting, do not change)
0xCA R/W 0x20 I2C addressing
Bit Function Default
0 reserved, I2C address A1, A0 of 7-bit I2C device address. Programmable only dur
ing reset via strap pins using external pull-up resistors.
0
1 0
2
I2C device address A6 … A2 of 7-bit I2C device address.
Programmable via direct access from I2C or from EEPROM during start up, fol
lowed by an I2C general call “Device address reload” to take it into effect.
0
3 0
4 0
5 1
6 0
7 reserved 0
0xCB R/W 0x03 I2C and TCMI control. Refer to chapter 13 and 6.4.
Bit Function Default
0 I2C clock stretching
0:  disabled
1:  enabled
1
1 I2C pins input spike filter
0:  disabled (> 1MHz)
1:  enabled (≤ 1MHz, FM+)
When I2C pins input spike filter = 0, SDA and SCL pins can be used up to 10MHz 
as inputs (driven rail-to-rail by a real CMOS driver, no pull-up) and up to 2MHz as 
outputs.
1
2, 3 reserved 0
4 00: Transfers 12 bit pixel data with 1x DCLK (default).
01: Transfers the 8 MSB bits of the pixel data with 1x DCLK. Data are LSB aligned.
10:  lsb/msb split mode: Transfers 12 bit pixel data with LSByte leading and MS
Byte trailing with 2x DCLK. Data are LSB aligned (default). The optional SAT bit is 
on the LSB.
11:  msb/lsb split mode: Transfers 12 bit pixel data with MSByte leading and LS
Byte trailing with 2x DCLK. Data are LSB aligned. The optional SAT bit is on the 
LSB.
0
5 0
6 When split modes selected (= 11 or 10), forces bit DATA[0] of the LSByte = 1 when
the pixel is saturated. Not effective with other TCMI data formats.
0:  disabled
1:  enabled
0
7 reserved 0
Cont. Table 36: Address map of RAM page (0x80 ~ 0xEF)
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
62 / 70 Datasheet_epc660-V2.20
www.espros.com
Addr. Type Default Description
0xCC R/W 0x41 TCMI polarity settings. Refer to chapter 6.4.
Bit Function Default
0 DCLK edge select to align all other TCMI outputs
0:  falling edge
1:  rising edge
1
1 HSYNC polarity
0:  HSYNC active low
1:  HSYNC active high
0
2 VSYNC polarity
0:  VSYNC active low
1:  VSYNC active high
0
3 XSYNC polarity
0:  XSYNC active low
1:  XSYNC active high
Only effective when bit 6 is set to 0
0
4 DATA[11:0] unsigned/signed TCMI data output format
0: unsigned integer, subtract 2'048 to get correct value (Default)
1: two's complement signed integer (-2'048 ... 2'047)
0
5 reserved 0
6 Select XSYNC / SAT output pin function
0:  XSYNC
1:  SAT
1
7 Force DATA[11:0] = 0xFFF (unsigned) / 0x7FF (signed, two's complement) during 
data-out operation when corresponding pixel is saturated
0:  disabled
1:  enabled
0
0xE8 R/W---Temperature offset correction for sensor top left. Value for calculation according the formula in chapter
10 by the application SW. Range approx. -27 … +27ºC with around 0.2ºC steps. The reference temper
ature is +27ºC. 0x7F (127) corresponds to 0ºC offset.  0xFF: Function is not supported.
0xAE0x
AE0xE9
R/W---DLL step. Supported for Wafer IDs 212 or higher. Refer for details to register 0x73 and Figure 23.
The exact value is tDLL = ((register 0xE9 -128) * 0.003ns) + 2.1ns (at +27ºC, VDD,VDDPLL = 1.8V).
0xFF: Function is not supported.
0xEA R/W---Temperature offset correction for sensor top right. Description see register 0xE8.
0xEC R/W---Temperature offset correction for sensor bottom left. Description see register 0xE8.
0xEE R/W---Temperature offset correction for sensor bottom right. Description see register 0xE8.
Cont. Table 36: Address map of RAM page (0x80 ~ 0xEF)
14.3. EEPROM page, indirect data access section (0xF0 ~ 0xFF)
Addr. Type Default Description
0xF0 R/W 0x00 User register for user data. Do not write the register during frame acquisition. The number of WRITE 
cycles into the EEPROM should not exceed 100 WRITE operations.
0xF5 R 0x00 Customer ID
0xF6 R---Wafer ID
0xF7 R--
0xF8 R---Chip ID
0xF9 R--
0xFA R 0x02 Chip and part type: 0x02 = epc660
0xFB R---Chip and part version (release) e.g. 0x07 for version -007, 0x0B for version -011
Table 37: Address map of EEPROM page (0xF0 ~ 0xFF)
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
63 / 70 Datasheet_epc660-V2.20
www.espros.com
15. Control command examples
15.1. I2C control command examples:
To simplify command sequence definitions, following C-programming language style functions are defined for the I2C master CPU:
■ i2cGeneralCall(byte genAdr, byte cmd);
■ i2cSingleWrite(byte devAdr, byte regAdr, byte regVal);
■ i2cMultiWrite(byte devAdr, byte regAdr, byte* regVal, byte n
■ byte i2cSingleRead(byte devAdr, byte regAdr);
■ byte* i2cMultiRead(byte devAdr, byte regAdr, byte n);
15.2. Software reset with I2C general call command
PRECONDITION: None
1.
i2cGeneralCall(0x00, 0x06);
2.
…
//  20 x tSCL = 20µs
//  29 x tSCL = 29µs
//  20 + (n x 9 x tSCL) = 20 + (n x 9)µs
//  39 x tSCL = 39µs
//  30 + (n x 9 x tSCL) = 30+(n x 9)µs
//  Software reset, same effect like RESET pin, 20µs
//  Wait for tRESET (> 100ns)
15.3. 4 DCS: Acquire DCS0 … 3 frames with tint = 16.6µs @ 12MHz modulation frequency
PRECONDITION: All other registers contain default values.
1.
i2cSingleWrite(0x20, 0x92, 0x34);
2.
3.
4.
i2cMultiWrite(0x20, 0xA2, &(0x031F), 2);
i2cSingleWrite(0x20, 0xA4, 0x01);
…
//  Modulation control 0x92 = 0x34 (mod. sel. = 00, No. DCS = 11), 29µs
//  Integration length 1 0xA2/0xA3 = 0x031F (integration time = 16.6µs), 38µs
//  Shutter control 0xA4 = 0x01, (shutter release = 1), 29µs
//  
Acquisition starts. Wait until all 4x DCS frames are finished.
15.4. 4 DCS: Acquire DCS0 … 3 frames with tint = 16.6µs, followed by DCS 0 … 3 with tint 333µs @ 12MHz mod. frequency
PRECONDITION: All other registers contain default values.
1.
i2cSingleWrite(0x20, 0x92, 0x34);
2.
3.
4.
5.
6.
7.
i2cMultiWrite(0x20, 0xA2, &(0x031F), 2);
i2cSingleWrite(0x20, 0xA4, 0x01);
…
i2cMultiWrite(0x20, 0xA2, &(0x3E7F), 2);
i2cSingleWrite(0x20, 0xA4, 0x01);
…
//  
//  
//  
//  
//  
//  
//  
Modulation control 0x92 = 0x34 (mod. sel. = 00, No. DCS = 11), 29µs
Integration length 1 0xA2/0xA3 = 0x031F (integration time = 16.6µs), 38µs
Shutter control 0xA4 = 0x01, (shutter release = 1), 29µs
Acquisition starts. Wait until all 4x DCS frames are finished.
Integration length 1 0xA2/0xA3 = 0x3E7F (integration time = 333µs), 38µs
Shutter control 0xA4 = 0x01, (shutter release = 1), 29µs
Acquisition starts. Wait until all 4x DCS frames are finished.
15.5. 2 DCS: Acquire DCS0 and 1 with tint = 16.6µs @ 12MHz modulation frequency
PRECONDITION: All other registers contain default values.
1.
i2cSingleWrite(0x20, 0x92, 0x14);
2.
3.
4.
i2cMultiWrite(0x20, 0xA2, &(0x031F), 2);
i2cSingleWrite(0x20, 0xA4, 0x01);
…
//  
//  
//  
//  
Modulation control 0x92 = 0x34 (mod. sel. = 00, No. DCS = 11), 29µs
Integration length 1 0xA2/0xA3 = 0x031F (integration time = 16.6µs), 38µs
Shutter control 0xA4 = 0x01, (shutter release = 1), 29µs
Acquisition starts. Wait until all 2x DCS frames are finished.
15.6. Indirect single write to EEPROM: Store 1 byte at user register 0xF0
PRECONDITION: None
1.
i2cSingleWrite(0x20, 0x11, 0xF0);
2.
3.
i2cSingleWrite(0x20, 0x12, 0x22);
...
//  EEPROM address register 0x11 = 0xF0, 29µs
//  EEPROM data register 0x12 =  0x22 
//  (user register = 0x22), 29µs + 20ms = ~20ms
Note 1: Start address is written in address register 0x11 for indirect read/write access to the EEPROM.
Note 2: Each EEPROM data register write starts erase/programming EEPROM. 
Each EEPROM write takes 20ms, then it auto-increments the EEPROM address register 0x11 by 1.
Note 3: Corresponding control register value is not modified. Only EEPROM register is modified.
Note 4: EEPROM content will only be copied to corresponding control register after RESET.
15.7. Indirect single read from EEPROM: Read 1 byte from user register 0xF0
PRECONDITION: None
1.
i2cSingleWrite(0x20, 0x11, 0xF0);
2.
3.
cal1 = i2cSingleRead(0x20, 0x12);
...
//  EEPROM address register 0x11 = 0xF0, 29µs
//  user value 1 = EEEPROM data register (user register 1 0xF0), 39µs
Note 1: Start address is written in the EEPROM address register 0x11.
Note 2: Corresponding control register value is not modified. Only EEPROM is read.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
64 / 70
Datasheet_epc660-V2.20
www.espros.com
15.8. Reading part version (register 0xFB)
Since there is no RAM register at address 0xFB, the PART VERSION can only be read directly from the EEPROM.
# The syntax of the I2C commands is as follows:
# Reading: i2c r REGISTER_ADDRESS [NUMBER_OF_BYTES]
# Writing: i2c w REGISTER_ADDRESS [DATA1 DATA2 ...]
i2c w 11 FB
i2c r 12 01  # Response: PART VERSION
15.9. Reading IC version (register 0x01)
I2C command to read IC version
# The syntax of the I2C commands is as follows:
# Reading: i2c r REGISTER_ADDRESS [NUMBER_OF_BYTES]
i2c r 01 01  # Response: IC VERSION
15.10. Reading WAFER ID and CHIP ID
It can be necessary for technical support to read the WAFER ID and the CHIP ID. Since there are no RAM register at addresses 0xF6 to
0xF9, the WAFER ID and the CHIP ID can only be read directly from the EEPROM.
# The syntax of the I2C commands is as follows:
# Reading: i2c r REGISTER_ADDRESS [NUMBER_OF_BYTES]
# Writing: i2c w REGISTER_ADDRESS [DATA1 DATA2 ...]
i2c w 11 F6
i2c r 12 01  # Response:  WAFER ID MSB
i2c r 12 01  # Response:  WAFER ID LSB
i2c r 12 01  # Response:  CHIP ID MSB
i2c r 12 01  # Response:  CHIP ID LSB
15.11. Pixel sequencer code write procedure
1. Startup epc660 chip (power up or reset release).
2. Wait until the chip is in READY state.
3. Write the pixel sequencer code from chapter 15.12 to the memory.
Important Notes:
This procedure has to be executed after every power up or after a chip reset release (refer also to chapter 11.1).
Never modify this code sequence. Otherwise malfunction occurs.
15.12. Pixel sequencer code
# Pixel Sequencer Code V14
# The following sequence of I2C commands re-program the sequencer to be on most actual functionality.
#
# The syntax of the I2C commands to the imager is as follows:
# Writing: i2c w REGISTER_ADDRESS [RAM_ADDRESS DATA0 DATA1 DATA2 DATA3 DATA4 DATA5 SR_PROGRAM]
i2c w a4 00 
i2c w 91 00 
i2c w 47 01 
i2c w 40 00 43 10 00 C0 00 00 0D 
i2c w 40 01 43 10 00 00 01 00 0D 
i2c w 40 02 43 10 00 40 0A 00 0D 
i2c w 40 03 43 10 10 02 58 00 0D 
i2c w 40 04 43 10 20 01 80 00 0D 
i2c w 40 05 43 10 F0 01 B0 00 0D 
i2c w 40 06 43 10 00 01 60 00 0D 
i2c w 40 07 43 10 C0 00 78 00 0D 
i2c w 40 08 43 10 40 00 18 00 0D 
i2c w 40 09 43 10 D0 02 40 00 0D 
i2c w 40 0A 43 10 10 C0 1E 00 0D 
i2c w 40 0B 43 10 00 00 50 00 0D 
i2c w 40 0C 43 10 20 00 18 00 0D 
i2c w 40 0D 43 10 D0 02 40 00 0D 
i2c w 40 0E 43 10 10 C0 1E 00 0D 
i2c w 40 0F 43 10 00 00 50 00 0D 
i2c w 40 10 43 10 D0 02 40 00 0D 
i2c w 40 11 43 10 00 00 50 00 0D 
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
65 / 70 Datasheet_epc660-V2.20
www.espros.com
i2c w 40 12 43 18 40 40 02 00 0D 
i2c w 40 13 43 08 02 00 00 00 0D 
i2c w 40 14 43 08 00 00 A8 00 0D 
i2c w 40 15 43 18 80 07 0C 00 0D 
i2c w 40 16 43 08 00 00 00 00 0D 
i2c w 40 17 43 08 01 00 00 00 0D 
i2c w 40 18 43 08 00 00 A8 00 0D 
i2c w 40 19 03 08 30 03 40 00 0D 
i2c w 40 1A 03 08 E0 01 60 00 0D 
i2c w 40 1B 03 08 10 C0 02 00 0D 
i2c w 40 1C 03 08 30 03 40 00 0D 
i2c w 40 1D 03 00 00 00 00 00 0D 
i2c w 40 1E 03 00 00 00 50 00 0D 
i2c w 40 1F 43 10 80 40 02 00 0D 
i2c w 40 20 43 10 60 00 50 00 0D 
i2c w 40 21 43 18 60 40 02 00 0D 
i2c w 40 22 43 18 90 07 0C 00 0D 
i2c w 40 23 43 08 01 00 00 00 0D 
i2c w 40 24 43 08 00 00 A8 00 0D 
i2c w 40 25 03 08 30 03 40 00 0D 
i2c w 40 26 03 00 88 00 10 00 0D 
i2c w 40 27 03 00 88 3E 0C 00 0D 
i2c w 40 28 03 00 08 00 14 00 0D 
i2c w 40 29 03 00 80 00 10 00 0D 
i2c w 40 2A 03 00 80 3E 0C 00 0D 
i2c w 40 2B 03 00 00 00 14 00 0D 
i2c w 40 2C 03 00 00 00 50 00 0D 
i2c w 40 2D 43 08 02 00 00 00 0D 
i2c w 40 2E 43 08 00 00 A8 00 0D 
i2c w 40 2F 43 18 00 00 00 00 0D 
i2c w 40 30 43 08 01 00 3C 00 0D 
i2c w 40 31 43 08 00 00 A8 00 0D 
i2c w 40 32 43 08 00 00 14 00 0D 
i2c w 40 33 43 08 00 C0 00 00 0D 
i2c w 40 34 43 08 00 00 01 00 0D 
i2c w 40 35 43 88 00 00 00 00 0D 
i2c w 40 36 43 08 30 0A 0C 00 0D 
i2c w 40 37 43 28 00 00 00 00 0D 
i2c w 40 38 43 08 40 00 0C 00 0D 
i2c w 40 39 43 08 C0 03 88 00 0D 
i2c w 40 3A 43 08 60 09 48 00 0D 
i2c w 40 3B 40 18 00 00 44 00 0D 
i2c w 40 3C 43 08 F0 03 8C 00 0D 
i2c w 40 3D 43 08 50 08 48 00 0D 
i2c w 40 3E 40 18 00 00 44 00 0D 
i2c w 40 3F 43 08 20 04 90 00 0D 
i2c w 40 40 41 08 60 07 48 00 0D 
i2c w 40 41 40 18 00 00 44 00 0D 
i2c w 40 42 41 08 40 04 48 00 0D 
i2c w 40 43 40 18 00 00 44 00 0D 
i2c w 40 44 05 08 00 00 34 00 0D 
i2c w 40 45 04 08 50 00 0C 00 0D 
i2c w 40 46 84 0A F0 00 0C 00 0D 
i2c w 40 47 84 0F 00 00 54 00 0D 
i2c w 40 48 85 0E 10 00 0C 00 0D 
i2c w 40 49 01 0E D0 00 0C 00 0D 
i2c w 40 4A 00 0E 00 00 AC 00 0D 
i2c w 40 4B 40 2E 00 00 00 00 0D 
i2c w 40 4C 40 08 80 05 9C 00 0D 
i2c w 40 4D 40 08 60 00 0C 00 0D 
i2c w 40 4E 41 08 00 00 00 00 0D 
i2c w 40 4F 09 48 00 00 00 00 0D 
i2c w 40 50 08 08 50 00 0C 00 0D 
i2c w 40 51 88 0A F0 00 0C 00 0D 
i2c w 40 52 88 0F 00 00 54 00 0D 
i2c w 40 53 89 0E 10 00 0C 00 0D 
i2c w 40 54 01 0E D0 00 0C 00 0D 
i2c w 40 55 00 0E 00 00 AC 00 0D 
i2c w 40 56 40 2E 00 00 00 00 0D 
i2c w 40 57 40 08 F0 06 94 00 0D 
i2c w 40 58 40 08 F0 06 94 00 0D 
i2c w 40 59 40 08 50 00 0C 00 0D 
i2c w 40 5A 41 08 00 00 00 00 0D 
i2c w 40 5B 11 48 00 00 00 00 0D 
i2c w 40 5C 10 08 50 00 0C 00 0D 
i2c w 40 5D 90 0A F0 00 0C 00 0D 
i2c w 40 5E 90 0F 00 00 54 00 0D 
i2c w 40 5F 91 0E 10 00 0C 00 0D 
i2c w 40 60 01 0E D0 00 0C 00 0D 
i2c w 40 61 00 0E 00 00 AC 00 0D 
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
66 / 70 Datasheet_epc660-V2.20
www.espros.com
i2c w 40 62 40 2E 00 00 00 00 0D 
i2c w 40 63 40 08 F0 06 9C 00 0D 
i2c w 40 64 40 08 60 00 0C 00 0D 
i2c w 40 65 41 08 00 00 00 00 0D 
i2c w 40 66 21 48 00 00 00 00 0D 
i2c w 40 67 20 08 50 00 0C 00 0D 
i2c w 40 68 A0 0A F0 00 0C 00 0D 
i2c w 40 69 A0 0F 00 00 54 00 0D 
i2c w 40 6A A1 0E 10 00 0C 00 0D 
i2c w 40 6B 01 0E D0 00 0C 00 0D 
i2c w 40 6C 00 0E 00 00 AC 00 0D 
i2c w 40 6D 40 2E 00 00 00 00 0D 
i2c w 40 6E 40 08 00 00 00 00 0D 
i2c w 40 6F 40 08 00 C0 03 00 0D 
i2c w 40 70 40 08 50 00 0C 00 0D 
i2c w 40 71 41 48 00 00 14 00 0D 
i2c w 40 72 00 08 00 00 54 00 0D 
i2c w 40 73 00 08 C0 02 0C 00 0D 
i2c w 40 74 00 58 00 00 00 00 0D 
i2c w 40 75 00 18 00 00 4C 00 0D 
i2c w 40 76 15 08 00 00 34 00 0D 
i2c w 40 77 14 08 50 00 0C 00 0D 
i2c w 40 78 94 0A F0 00 0C 00 0D 
i2c w 40 79 94 0F 00 00 54 00 0D 
i2c w 40 7A 95 0E 10 00 0C 00 0D 
i2c w 40 7B 01 0E D0 00 0C 00 0D 
i2c w 40 7C 00 0E 00 00 AC 00 0D 
i2c w 40 7D 40 2E 00 00 00 00 0D 
i2c w 40 7E 40 08 70 00 0C 00 0D 
i2c w 40 7F 41 08 00 00 00 00 0D 
i2c w 40 80 29 48 00 00 00 00 0D 
i2c w 40 81 28 08 50 00 0C 00 0D 
i2c w 40 82 A8 0A F0 00 0C 00 0D 
i2c w 40 83 A8 0F 00 00 54 00 0D 
i2c w 40 84 A9 0E B0 06 50 00 0D 
i2c w 40 85 41 08 00 00 00 00 0D 
i2c w 40 86 0D 08 00 00 34 00 0D 
i2c w 40 87 0C 08 50 00 0C 00 0D 
i2c w 40 88 8C 0A F0 00 0C 00 0D 
i2c w 40 89 8C 0F 00 00 54 00 0D 
i2c w 40 8A 8D 0E 10 00 0C 00 0D 
i2c w 40 8B 01 0E D0 00 0C 00 0D 
i2c w 40 8C 00 0E 00 00 AC 00 0D 
i2c w 40 8D 40 2E 00 00 00 00 0D 
i2c w 40 8E 40 08 F0 06 94 00 0D 
i2c w 40 8F 40 08 60 00 0C 00 0D 
i2c w 40 90 41 08 00 00 00 00 0D 
i2c w 40 91 31 48 00 00 00 00 0D 
i2c w 40 92 30 08 50 00 0C 00 0D 
i2c w 40 93 B0 0A F0 00 0C 00 0D 
i2c w 40 94 B0 0F 00 00 54 00 0D 
i2c w 40 95 B1 0E B0 06 50 00 0D 
i2c w 40 96 43 08 00 00 00 00 0D 
i2c w 40 97 41 08 00 00 00 00 0D 
i2c w 40 98 3D 08 00 00 34 00 0D 
i2c w 40 99 3C 08 50 00 0C 00 0D 
i2c w 40 9A BC 0A F0 00 0C 00 0D 
i2c w 40 9B BC 0F 00 00 54 00 0D 
i2c w 40 9C BD 0E B0 06 50 00 0D 
i2c w 47 00 
i2c w 91 03
It is possible to read the sequencer code back from memory. This is useful to ensure that the sequencer code is correctly stored and was
not accidentally changed during operation.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
67 / 70 Datasheet_epc660-V2.20
www.espros.com
15.13. Pixel sequencer code read back
I2C command to imager description / comment
i2c w a4 00 #disable acquisition
i2c w 91 00 #stop sequencer
i2c w 40 00
i2c w 47 09
Data0 = i2c r 41
Data1 = i2c r 42
Data2 = i2c r 43
Data3 = i2c r 44
Data4 = i2c r 45
Data5 = i2c r 46
#set dedicated sequencer RAM address (e.g. address 0x00)
#enable pixel sequencer RAM access
i2c w 40 01
i2c w 47 09
Data0 = i2c r 41
Data1 = i2c r 42
Data2 = i2c r 43
Data3 = i2c r 44
Data4 = i2c r 45
Data5 = i2c r 46
#set dedicated sequencer RAM address (e.g. address 0x01)
#enable pixel sequencer RAM access
… …
i2c w 47 00 #disable pixel sequencer RAM access
i2c w 91 03 #start sequencer
Read back results (Sequencer V11)
RAM address
pixel sequencer
Data0 Data1 Data2 Data3 Data4 Data5
0x00 0x43 0x10 0x00 0xC0 0x00 0x00
0x01 0x43 0x10 0x00 0x00 0x01 0x00
... ... ... ... ... ... ...
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
68 / 70 Datasheet_epc660-V2.20
www.espros.com
16. Addendum
16.1. Terms, definitions and abbreviations
Abbreviation Term, Definition Explanation
ABS Automatic Backlight Suppression
ADC Analog Digital Converter
AMR Ambient-light to Modulated light ratio
CGU Clock Generation Unit
CSP Chip Scale Package
DCS Differential Correlation Sample
DLL Delay Locked Loop Delay line only in the implementation of epc660
fps Frames per second
Half-QQVGA 1/8 of a Quarter VGA 160x60 pixel resolution
HDR High Dynamic Range
IC Integrated Circuit
LED/LD Light Emitting Diode / Laser Diode
LSB Least Significant Bit
MGA Modulation Gate A
MGB Modulation Gate B
MGX Modulation Gate A or B
mga MGA control signal
mgb MGB control signal
mgx MGX control signal
MSB Most Significant Bit
OSC Oscillator
PLL Phase Locked Loop
ROI Region of Interest
QVGA Quarter VGA 320x240 pixel resolution
SGA Storage Gate A
SGB Storage Gate B
SGX Storage Gate A or B
TCMI TOF Camera Module Interface
TOF Time of Flight
VGA Video Graphics Array 640x480 pixel resolution
XTAL Crystal
Table 38: Definitions and abbreviations
16.2. Related documents
■3D-TOF, A guideline to 3D-TOF sensors that work, Beat De Coi, ISBN 978-3-033-07096-7.
■Application note AN08 Process-Rules CSP Assembly, ESPROS Photonics corp.
■Application note AN10 Calibration and compensation of Cameras using ESPROS TOF Chips, ESPROS Photonics corp.
■Application note AN11 DME 660 Photobiological Safety Analysis, ESPROS Photonics Corp.
■Application note AN12 TOF data improvement toolbox, ESPROS Photonics Corp.
■NXP I2C-bus specification: I2C Bus Specification and User Manual, NXP corp.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
69 / 70 Datasheet_epc660-V2.20
www.espros.com
17. IMPORTANT NOTICE
ESPROS Photonics AG and its subsidiaries (ESPROS) reserve the right to make corrections, modifications, enhancements, improve
ments, and other changes to its products and services at any time and to discontinue any product or service without notice. Customers
should obtain the latest relevant information before placing orders and should verify that such information is current and complete. All prod
ucts are sold subject to ESPROS' terms and conditions of sale supplied at the time of order acknowledgment.
ESPROS warrants performance of its hardware products to the specifications applicable at the time of sale in accordance with ESPROS'
standard warranty. Testing and other quality control techniques are used to the extent ESPROS deems necessary to support this warranty.
Except where mandated by government requirements, testing of all parameters of each product is not necessarily performed.
ESPROS assumes no liability for applications assistance or customer product design. Customers are responsible for their products and
applications using ESPROS components. To minimize the risks associated with customer products and applications, customers should
provide adequate design and operating safeguards.
ESPROS does not warrant or represent that any license, either express or implied, is granted under any ESPROS patent right, copyright,
mask work right, or other ESPROS intellectual property right relating to any combination, machine, or process in which ESPROS products
or services are used. Information published by ESPROS regarding third-party products or services does not constitute a license from ES
PROS to use such products or services or a warranty or endorsement thereof. Use of such information may require a license from a third
party under the patents or other intellectual property of the third party, or a license from ESPROS under the patents or other intellectual
property of ESPROS.
Resale of ESPROS products or services with statements different from or beyond the parameters stated by ESPROS for that product or
service voids all express and any implied warranties for the associated ESPROS product or service. ESPROS is not responsible or liable
for any such statements.
ESPROS products are not authorized for use in safety-critical applications (such as life support) where a failure of the ESPROS product
would reasonably be expected to cause severe personal injury or death, unless officers of the parties have executed an agreement specifi
cally governing such use. Buyers represent that they have all necessary expertise in the safety and regulatory ramifications of their appli
cations, and acknowledge and agree that they are solely responsible for all legal, regulatory and safety-related requirements concerning
their products and any use of ESPROS products in such safety-critical applications, notwithstanding any applications-related information or
support that may be provided by ESPROS. Further, Buyers must fully indemnify ESPROS and its representatives against any damages
arising out of the use of ESPROS products in such safety-critical applications.
ESPROS products are neither designed nor intended for use in military/aerospace applications or environments unless the ESPROS prod
ucts are specifically designated by ESPROS as military-grade. Only products designated by ESPROS as military-grade meet military spec
ifications. Buyers acknowledge and agree that any such use of ESPROS products which ESPROS has not designated as military-grade is
solely at the Buyer's risk, and that they are solely responsible for compliance with all legal and regulatory requirements in connection with
such use.
ESPROS products are neither designed nor intended for use in automotive applications or environments unless the specific ESPROS
products are designated by ESPROS as compliant with ISO/TS 16949 requirements. Buyers acknowledge and agree that, if they use any
non-designated products in automotive applications, ESPROS will not be responsible for any failure to meet such requirements.
© 2023 ESPROS Photonics Corporation
Characteristics subject to change without notice
70 / 70
Datasheet_epc660-V2.20
www.espros.com