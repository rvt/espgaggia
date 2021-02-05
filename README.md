[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# Design Goal
A make a gaggia coffee machine more smart and make it such that is would be easy to duplicate with least amouth of soldering.

Features
* Control temperature of both stream and brew and set temperature with a touch display (Fuzzy Logic)
* pre-infuse your coffee
* Make recipes using a simple scripting language with menu's controle brew/preinfuse timings and temperatures
* Standby + Remote turn on (Using MQTT)
* Deep sleep option (work in progress)
* Stand by option with a lower temperature (work in progress)


Main screen:

![images](images/screen1.jpg "Screen 1")


Recipe/Menu screen:

![images](images/screen2.jpg "Screen 1")

Settings Screen:

![images](images/screen3.jpg "Screen 1")

# Hardware Tests

* Tested with ESP 32 and ILI9341 touch display

# Word Of Warning

### Please read this carefull!

If you are going to build this project you have to understand you will be working with dangerous voltages and high temperatures! Although I did my best to use standard components that I think are reliable, I cannot be held responsible for any damages to your machine, yourself or the surroundings in any way possible.

# Compilation

``` bash
rvt$ pio run
Processing gaggia (platform: espressif32; framework: arduino; board: esp32dev)
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Verbose mode can be enabled via `-v, --verbose` option
CONFIGURATION: https://docs.platformio.org/page/boards/espressif32/esp32dev.html
PLATFORM: Espressif 32 (2.1.0) > Espressif ESP32 Dev Module
...
...
...
|-- <WiFi> 1.0
Building in debug mode
Retrieving maximum program size .pio/build/gaggia/firmware.elf
Checking size .pio/build/gaggia/firmware.elf
Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
RAM:   [===       ]  25.6% (used 84048 bytes from 327680 bytes)
Flash: [=======   ]  72.6% (used 1427385 bytes from 1966080 bytes)
============== [SUCCESS] Took 4.41 seconds ==============
```

To upload to your wemos device run the following command (OSX):
```platformio run --target upload -e gaggia```
```platformio run --target uploadfs -e gaggia```

For other compiler options check `config.hpp` and `platformio.ini` for other options. 

# Configuration

The device will use the know wifi network or it wil present itself as a WIFI accesspoint using a wifimanager. Just connect to that accesspoint and open a browser to configure WIFI andf MQTT.
By default it will have some scripts to do some of it's work. Check the data directory that is almost self explanatory on how it works.

# MQTT Messages

Will be written soon

# Scripting

Will be written soon and how to add your own menu entries

# Hardware and Connection

* 1x ESP32 
* 2x Zero Crossing SSR 10Amp (DC-AC) https://nl.aliexpress.com/item/32706812752.html?spm=a2g0s.9042311.0.0.46b54c4dXPbjTF
* 1x Zero Crossing SSR 25AMP (DC-AC)  https://nl.aliexpress.com/item/32706812752.html?spm=a2g0s.9042311.0.0.46b54c4dXPbjTF
* 2x Probe M4 Type K temperature sensor  https://nl.aliexpress.com/item/32824035575.html?spm=a2g0s.9042311.0.0.46b54c4delJT6H
* 1x ILI9341 + Touch display 240x320 2.8" SPI TFT LCD Touch Panel https://nl.aliexpress.com/item/4000631140288.html?spm=a2g0s.9042311.0.0.46b54c4delJT6H
* 2x MAX31855K K Type Thermokoppel Breakout Board  https://nl.aliexpress.com/item/4000030496237.html?spm=a2g0o.cart.0.0.26e83c00vfImW6&mp=1
* Assorted silicon wires that can handle the current, like XXAWG for the heat element (for 230V Gaggia), the rest can be XXAWG 
* HLK-PM01 230 -> 5VDC power supply https://nl.aliexpress.com/item/32408565688.html?spm=a2g0s.9042311.0.0.13984c4dg0pb5v

note: For the MAX31855K make sure you get the 'fancy' one, that is proper dialectric consensator, NOT the one with just two components

# Connection diagram

For pins in `platformio.ini`

_WARNING schematic needs further verification_

![images](images/schematic.jpg "Screen 1")

# Hardware Tested

- [x] TFT/Touch display (ILI9341)
- [x] Temperature sensors
- [ ] Buttons
- [ ] SSR
- [x] Temperature control (on other project verified)
