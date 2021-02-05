EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L RF_Module:ESP32-WROOM-32 U7
U 1 1 601DA369
P 1700 3400
F 0 "U7" H 1250 4750 50  0000 C CNN
F 1 "ESP32-WROOM-32" H 2100 4750 50  0000 C CNN
F 2 "RF_Module:ESP32-WROOM-32" H 1700 1900 50  0001 C CNN
F 3 "https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf" H 1400 3450 50  0001 C CNN
	1    1700 3400
	1    0    0    -1  
$EndComp
$Comp
L Relay_SolidState:S216S02 U5
U 1 1 601DF7DB
P 6600 2150
F 0 "U5" H 6600 2475 50  0000 C CNN
F 1 "S216S02" H 6600 2384 50  0000 C CNN
F 2 "Package_SIP:SIP4_Sharp-SSR_P7.62mm_Straight" H 6400 1950 50  0001 L CIN
F 3 "http://www.sharp-world.com/products/device/lineup/data/pdf/datasheet/s116s02_e.pdf" H 6600 2150 50  0001 L CNN
	1    6600 2150
	1    0    0    -1  
$EndComp
$Comp
L Relay_SolidState:S202S02 U3
U 1 1 601E0DE1
P 6600 1000
F 0 "U3" H 6600 1325 50  0000 C CNN
F 1 "S202S02" H 6600 1234 50  0000 C CNN
F 2 "Package_SIP:SIP4_Sharp-SSR_P7.62mm_Straight" H 6400 800 50  0001 L CIN
F 3 "http://www.sharp-world.com/products/device/lineup/data/pdf/datasheet/s102s02_e.pdf" H 6600 1000 50  0001 L CNN
	1    6600 1000
	1    0    0    -1  
$EndComp
$Comp
L Relay_SolidState:S202S02 U4
U 1 1 601E1DED
P 6600 1550
F 0 "U4" H 6600 1875 50  0000 C CNN
F 1 "S202S02" H 6600 1784 50  0000 C CNN
F 2 "Package_SIP:SIP4_Sharp-SSR_P7.62mm_Straight" H 6400 1350 50  0001 L CIN
F 3 "http://www.sharp-world.com/products/device/lineup/data/pdf/datasheet/s102s02_e.pdf" H 6600 1550 50  0001 L CNN
	1    6600 1550
	1    0    0    -1  
$EndComp
$Comp
L Sensor_Temperature:MAX31855KASA U1
U 1 1 601EB9F5
P 6650 4800
F 0 "U1" H 6400 5150 50  0000 C CNN
F 1 "MAX31855KASA" H 6950 5150 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 7650 4450 50  0001 C CIN
F 3 "http://datasheets.maximintegrated.com/en/ds/MAX31855.pdf" H 6650 4800 50  0001 C CNN
	1    6650 4800
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR017
U 1 1 601F3646
P 6650 5200
F 0 "#PWR017" H 6650 4950 50  0001 C CNN
F 1 "GNDD" H 6654 5045 50  0000 C CNN
F 2 "" H 6650 5200 50  0001 C CNN
F 3 "" H 6650 5200 50  0001 C CNN
	1    6650 5200
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR011
U 1 1 601F6D57
P 6300 900
F 0 "#PWR011" H 6300 750 50  0001 C CNN
F 1 "+3V3" H 6315 1073 50  0000 C CNN
F 2 "" H 6300 900 50  0001 C CNN
F 3 "" H 6300 900 50  0001 C CNN
	1    6300 900 
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR012
U 1 1 601F74D5
P 6300 1450
F 0 "#PWR012" H 6300 1300 50  0001 C CNN
F 1 "+3V3" H 6315 1623 50  0000 C CNN
F 2 "" H 6300 1450 50  0001 C CNN
F 3 "" H 6300 1450 50  0001 C CNN
	1    6300 1450
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR013
U 1 1 601F9AB5
P 6300 2050
F 0 "#PWR013" H 6300 1900 50  0001 C CNN
F 1 "+3V3" H 6315 2223 50  0000 C CNN
F 2 "" H 6300 2050 50  0001 C CNN
F 3 "" H 6300 2050 50  0001 C CNN
	1    6300 2050
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR016
U 1 1 60200FE5
P 6650 4400
F 0 "#PWR016" H 6650 4250 50  0001 C CNN
F 1 "+3V3" H 6665 4573 50  0000 C CNN
F 2 "" H 6650 4400 50  0001 C CNN
F 3 "" H 6650 4400 50  0001 C CNN
	1    6650 4400
	1    0    0    -1  
$EndComp
$Comp
L power:+5VD #PWR02
U 1 1 602AA45D
P 1700 2000
F 0 "#PWR02" H 1700 1850 50  0001 C CNN
F 1 "+5VD" H 1715 2173 50  0000 C CNN
F 2 "" H 1700 2000 50  0001 C CNN
F 3 "" H 1700 2000 50  0001 C CNN
	1    1700 2000
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR014
U 1 1 60201C74
P 6650 3050
F 0 "#PWR014" H 6650 2900 50  0001 C CNN
F 1 "+3V3" H 6650 3200 50  0000 C CNN
F 2 "" H 6650 3050 50  0001 C CNN
F 3 "" H 6650 3050 50  0001 C CNN
	1    6650 3050
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR015
U 1 1 601F143B
P 6650 3850
F 0 "#PWR015" H 6650 3600 50  0001 C CNN
F 1 "GNDD" H 6654 3695 50  0000 C CNN
F 2 "" H 6650 3850 50  0001 C CNN
F 3 "" H 6650 3850 50  0001 C CNN
	1    6650 3850
	1    0    0    -1  
$EndComp
$Comp
L Sensor_Temperature:MAX31855KASA U2
U 1 1 601ED58A
P 6650 3450
F 0 "U2" H 6400 3800 50  0000 C CNN
F 1 "MAX31855KASA" H 6950 3800 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 7650 3100 50  0001 C CIN
F 3 "http://datasheets.maximintegrated.com/en/ds/MAX31855.pdf" H 6650 3450 50  0001 C CNN
	1    6650 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	7300 3350 7050 3350
Wire Wire Line
	7050 3550 7500 3550
$Comp
L Converter_ACDC:HLK-PM01 PS1
U 1 1 6030A3A6
P 3050 6150
F 0 "PS1" H 3050 6475 50  0000 C CNN
F 1 "HLK-PM01" H 3050 6384 50  0000 C CNN
F 2 "Converter_ACDC:Converter_ACDC_HiLink_HLK-PMxx" H 3050 5850 50  0001 C CNN
F 3 "http://www.hlktech.net/product_detail.php?ProId=54" H 3450 5800 50  0001 C CNN
	1    3050 6150
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR06
U 1 1 6030D498
P 3450 6250
F 0 "#PWR06" H 3450 6000 50  0001 C CNN
F 1 "GNDD" H 3454 6095 50  0000 C CNN
F 2 "" H 3450 6250 50  0001 C CNN
F 3 "" H 3450 6250 50  0001 C CNN
	1    3450 6250
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR03
U 1 1 6030EA87
P 1700 4800
F 0 "#PWR03" H 1700 4550 50  0001 C CNN
F 1 "GNDD" H 1704 4645 50  0000 C CNN
F 2 "" H 1700 4800 50  0001 C CNN
F 3 "" H 1700 4800 50  0001 C CNN
	1    1700 4800
	1    0    0    -1  
$EndComp
Text Notes 7900 3450 0    197  ~ 0
Steam
Text Notes 7950 4800 0    197  ~ 0
Brew
Text Notes 7900 2300 0    197  ~ 0
Boiler
Text Notes 7900 1150 0    197  ~ 0
Valve
Text Notes 7900 1700 0    197  ~ 0
Pump
$Comp
L Connector:Screw_Terminal_01x02 J1
U 1 1 60320D85
P 4150 3200
F 0 "J1" H 4230 3192 50  0000 L CNN
F 1 "Screw_Terminal_01x02" H 4230 3101 50  0000 L CNN
F 2 "" H 4150 3200 50  0001 C CNN
F 3 "~" H 4150 3200 50  0001 C CNN
	1    4150 3200
	1    0    0    -1  
$EndComp
$Comp
L Connector:Screw_Terminal_01x02 J2
U 1 1 60322862
P 4150 3550
F 0 "J2" H 4230 3542 50  0000 L CNN
F 1 "Screw_Terminal_01x02" H 4230 3451 50  0000 L CNN
F 2 "" H 4150 3550 50  0001 C CNN
F 3 "~" H 4150 3550 50  0001 C CNN
	1    4150 3550
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR08
U 1 1 60323735
P 3950 3650
F 0 "#PWR08" H 3950 3400 50  0001 C CNN
F 1 "GNDD" H 3954 3495 50  0000 C CNN
F 2 "" H 3950 3650 50  0001 C CNN
F 3 "" H 3950 3650 50  0001 C CNN
	1    3950 3650
	1    0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR07
U 1 1 60324341
P 3950 3300
F 0 "#PWR07" H 3950 3050 50  0001 C CNN
F 1 "GNDD" H 3954 3145 50  0000 C CNN
F 2 "" H 3950 3300 50  0001 C CNN
F 3 "" H 3950 3300 50  0001 C CNN
	1    3950 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 3550 3650 3550
$Comp
L Connector:Screw_Terminal_01x02 J3
U 1 1 6033641C
P 1950 6150
F 0 "J3" H 1868 6367 50  0000 C CNN
F 1 "Screw_Terminal_01x02" H 1868 6276 50  0000 C CNN
F 2 "" H 1950 6150 50  0001 C CNN
F 3 "~" H 1950 6150 50  0001 C CNN
	1    1950 6150
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2650 6050 2450 6050
Wire Wire Line
	2450 6050 2450 6150
Wire Wire Line
	2450 6150 2150 6150
Wire Wire Line
	2650 6250 2150 6250
$Comp
L power:+5VD #PWR05
U 1 1 60344957
P 3450 6050
F 0 "#PWR05" H 3450 5900 50  0001 C CNN
F 1 "+5VD" H 3465 6223 50  0000 C CNN
F 2 "" H 3450 6050 50  0001 C CNN
F 3 "" H 3450 6050 50  0001 C CNN
	1    3450 6050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3650 3550 3650 3300
Wire Wire Line
	3650 3300 2300 3300
$Comp
L Connector:Conn_01x04_Male J5
U 1 1 60351F72
P 4150 2300
F 0 "J5" H 4300 2550 50  0000 R CNN
F 1 "Conn_01x04_Male" H 4500 2050 50  0000 R CNN
F 2 "" H 4150 2300 50  0001 C CNN
F 3 "~" H 4150 2300 50  0001 C CNN
	1    4150 2300
	-1   0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x04_Male J6
U 1 1 60352B1A
P 2250 1200
F 0 "J6" H 2400 1450 50  0000 R CNN
F 1 "Conn_01x04_Male" H 2600 950 50  0000 R CNN
F 2 "" H 2250 1200 50  0001 C CNN
F 3 "~" H 2250 1200 50  0001 C CNN
	1    2250 1200
	-1   0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x04_Male J4
U 1 1 60353A7F
P 1300 1200
F 0 "J4" H 1450 1450 50  0000 R CNN
F 1 "Conn_01x04_Male" H 1650 950 50  0000 R CNN
F 2 "" H 1300 1200 50  0001 C CNN
F 3 "~" H 1300 1200 50  0001 C CNN
	1    1300 1200
	-1   0    0    -1  
$EndComp
$Comp
L power:GNDD #PWR01
U 1 1 6035A367
P 1000 1500
F 0 "#PWR01" H 1000 1250 50  0001 C CNN
F 1 "GNDD" H 1004 1345 50  0000 C CNN
F 2 "" H 1000 1500 50  0001 C CNN
F 3 "" H 1000 1500 50  0001 C CNN
	1    1000 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	1100 1100 1000 1100
Wire Wire Line
	1000 1100 1000 1200
Wire Wire Line
	1100 1200 1000 1200
Connection ~ 1000 1200
Wire Wire Line
	1000 1200 1000 1300
Wire Wire Line
	1100 1300 1000 1300
Connection ~ 1000 1300
Wire Wire Line
	1000 1300 1000 1400
Wire Wire Line
	1100 1400 1000 1400
Connection ~ 1000 1400
Wire Wire Line
	1000 1400 1000 1500
$Comp
L power:+3V3 #PWR04
U 1 1 603663A8
P 1950 900
F 0 "#PWR04" H 1950 750 50  0001 C CNN
F 1 "+3V3" H 1965 1073 50  0000 C CNN
F 2 "" H 1950 900 50  0001 C CNN
F 3 "" H 1950 900 50  0001 C CNN
	1    1950 900 
	1    0    0    -1  
$EndComp
Wire Wire Line
	2050 1400 1950 1400
Wire Wire Line
	1950 1400 1950 1300
Wire Wire Line
	2050 1300 1950 1300
Connection ~ 1950 1300
Wire Wire Line
	1950 1300 1950 1200
Wire Wire Line
	2050 1200 1950 1200
Connection ~ 1950 1200
Wire Wire Line
	1950 1200 1950 1100
Wire Wire Line
	2050 1100 1950 1100
Connection ~ 1950 1100
Wire Wire Line
	1950 1100 1950 900 
Wire Wire Line
	3950 2500 3300 2500
Wire Wire Line
	3300 2500 3300 4500
Wire Wire Line
	3300 4500 2300 4500
Wire Wire Line
	3200 4400 2300 4400
Wire Wire Line
	2300 2900 3100 2900
Wire Wire Line
	3100 2900 3100 2300
Text Notes 5200 3350 0    100  ~ 0
Brew
Text Notes 5200 3700 0    100  ~ 0
Steam
$Comp
L Connector:Conn_01x14_Male J7
U 1 1 603B5B34
P 4400 1350
F 0 "J7" H 4508 2131 50  0000 C CNN
F 1 "Conn_01x14_Male" H 4508 2040 50  0000 C CNN
F 2 "" H 4400 1350 50  0001 C CNN
F 3 "~" H 4400 1350 50  0001 C CNN
	1    4400 1350
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 750  4800 750 
Wire Wire Line
	4600 850  4800 850 
Wire Wire Line
	4600 950  4800 950 
Wire Wire Line
	4600 1050 4800 1050
Wire Wire Line
	4600 1150 4800 1150
Wire Wire Line
	4600 1250 4800 1250
Wire Wire Line
	4600 1350 4800 1350
Wire Wire Line
	4600 1450 4800 1450
Wire Wire Line
	4600 1550 4800 1550
Wire Wire Line
	4600 1650 4800 1650
Wire Wire Line
	4600 1750 4800 1750
Wire Wire Line
	4600 1850 4800 1850
Wire Wire Line
	4600 1950 4800 1950
Wire Wire Line
	4600 2050 4800 2050
Text Label 4800 950  0    50   ~ 0
dsp_cs
Text Label 4800 1050 0    50   ~ 0
dsp_rst
Text Label 4800 1150 0    50   ~ 0
dsp_dc
Text Label 4800 1250 0    50   ~ 0
dsp_mosi
Text Label 4800 1350 0    50   ~ 0
dsp_ck
Text Label 4800 1450 0    50   ~ 0
dsp_led
Text Label 4800 1550 0    50   ~ 0
dsp_miso
Text Label 4800 1650 0    50   ~ 0
dsp_ck
Text Label 4800 1750 0    50   ~ 0
tft_cs
Text Label 4800 1850 0    50   ~ 0
dsp_mosi
Text Label 4800 1950 0    50   ~ 0
dsp_miso
$Comp
L power:GNDD #PWR010
U 1 1 6040DC01
P 4800 850
F 0 "#PWR010" H 4800 600 50  0001 C CNN
F 1 "GNDD" V 4804 740 50  0000 R CNN
F 2 "" H 4800 850 50  0001 C CNN
F 3 "" H 4800 850 50  0001 C CNN
	1    4800 850 
	0    -1   -1   0   
$EndComp
$Comp
L power:+5VD #PWR09
U 1 1 6040E749
P 4800 750
F 0 "#PWR09" H 4800 600 50  0001 C CNN
F 1 "+5VD" V 4815 878 50  0000 L CNN
F 2 "" H 4800 750 50  0001 C CNN
F 3 "" H 4800 750 50  0001 C CNN
	1    4800 750 
	0    1    1    0   
$EndComp
Text Label 4800 2050 0    50   ~ 0
tft_irq
Wire Wire Line
	2300 3500 2550 3500
Wire Wire Line
	2300 3400 2550 3400
Wire Wire Line
	2300 3100 2550 3100
Wire Wire Line
	2300 2700 2550 2700
Wire Wire Line
	2300 2600 2550 2600
Wire Wire Line
	2300 3800 2550 3800
Wire Wire Line
	2300 3600 2550 3600
Wire Wire Line
	2300 4000 2550 4000
Text Label 2550 4000 0    50   ~ 0
tft_irq
Text Label 2550 3500 0    50   ~ 0
dsp_miso
Text Label 2550 3800 0    50   ~ 0
dsp_mosi
Text Label 2550 3100 0    50   ~ 0
tft_cs
Text Label 2550 3400 0    50   ~ 0
dsp_ck
Text Label 2550 3600 0    50   ~ 0
dsp_led
Text Label 2550 2600 0    50   ~ 0
dsp_rst
Text Label 2550 2700 0    50   ~ 0
dsp_cs
Text Label 2550 3000 0    50   ~ 0
temp_ck
Text Label 2550 2800 0    50   ~ 0
temp_do
Wire Wire Line
	2300 2800 2550 2800
Wire Wire Line
	2550 3000 2300 3000
Text Label 7500 3250 0    50   ~ 0
tft_cs
Text Label 7500 3350 0    50   ~ 0
tft_do
Wire Wire Line
	7400 3250 7400 4700
Wire Wire Line
	7050 3250 7400 3250
Wire Wire Line
	7500 3250 7400 3250
Connection ~ 7400 3250
Wire Wire Line
	7500 3350 7300 3350
Connection ~ 7300 3350
Wire Wire Line
	7050 4600 7300 4600
Wire Wire Line
	7050 4700 7400 4700
Wire Wire Line
	7050 4900 7500 4900
Text Label 7500 4900 0    50   ~ 0
temp_brew_cs
Text Label 7500 3550 0    50   ~ 0
temp_ste_cs
Wire Wire Line
	2300 4100 2550 4100
Wire Wire Line
	2300 3700 2550 3700
Text Label 2550 3700 0    50   ~ 0
temp_ste_cs
Text Label 2550 4100 0    50   ~ 0
temp_brew_cs
Wire Wire Line
	2300 3200 3950 3200
Wire Wire Line
	2300 3900 2550 3900
Wire Wire Line
	2300 4200 2550 4200
Wire Wire Line
	2300 4300 2550 4300
Wire Wire Line
	6300 1100 6150 1100
Wire Wire Line
	6300 1650 6150 1650
Wire Wire Line
	6300 2250 6150 2250
Text Label 6150 1100 2    50   ~ 0
ssr_valve
Text Label 6150 1650 2    50   ~ 0
ssr_pump
Text Label 6150 2250 2    50   ~ 0
ssr_boiler
Wire Wire Line
	7300 3350 7300 4600
Wire Wire Line
	2300 2400 2550 2400
Text Label 2550 2400 0    50   ~ 0
dsp_dc
Text Label 2550 4200 0    50   ~ 0
ssr_pump
Text Label 2550 4300 0    50   ~ 0
ssr_pump
Text Label 2550 3900 0    50   ~ 0
ssr_valve
Wire Wire Line
	2300 2200 3950 2200
Wire Wire Line
	3950 2300 3100 2300
Wire Wire Line
	3950 2400 3200 2400
Wire Wire Line
	3200 2400 3200 4400
$EndSCHEMATC
