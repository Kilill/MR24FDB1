
# MR24FDB1 driver class

First this is very much a work in progress, so absoloutly no guarantees..

The MR24FDB1 is a 25Ghz presence detector radar by [Seed Studio](https://www.seeedstudio.com/24GHz-mmWave-Radar-Sensor-Fall-Detection-Module-p-5268.html)

I found the lack of a proper driver frustrating and the suplied documentation equally confusing
so in my way to try to undrstand how the module worked and what the documentation actually meant
i embarked on this project.

Underway i have renamed  some (most) of the labels that Seed Studio has in their documentation to something that i could wrap
my head around, this might or might not make it clearer how the API the module presents work.
il try [bracket] the original names/definitions in the text.

The MR23FDB1 class is able to read and decode the reports [Proactive reports] that the module continously sends out on its serial interface.
It can also send out request for data [Read Command] the and parse the responces [Passive Reports].

Currently written for ESP32/Arduino framwork using platformio.

And yes i use "TAB's" instead of "SPACE" to indent. Thats what the character is for, and most modern editors are capable off setting the indent
amount that a tab character indents.

# Known and/or uknown stuff

Not all report functions and data are documented in the documentation (or acording to me correctly named/documented):

1. There is 2 versions of the documentation from SeedStudio the original unmarked one
	and a later one marked with V1.7. The the later does not document the fall reporting, 
	plus some other  minor differences.

2. Proactive report (0x04) / Other Information (0x03) / 0x09 (upgrade packet ? ) 
  Vill once in a wile be reported , this is not documented as an report [Proactive report] function.
  It does exist as a Responce [Passive Report] but then in conjunction with ota upgrade.

3. Once in a while the modules seems to miss a command and never sends the proper responce.

4. after a reboot the module is supoed to send an init success report this does not happen.


# Dependencies

This uses my debug library: github.com:Kilill/debug.git to print out errors, warnings
debug messages etc.
 
# Example

The example directory contains an example platformio project that initialzes the module,
sets some parameters and continously queries it for presence,location,heartbeat and movement

# Docs

The doc directory contains the modified documentation as LibreOffice calc file, the same
as a PDF

# File structure
```
├── decodeError.cpp
├── doc
│   ├── dia										Dia diagram files over the protoco
│   ├── doxypress.json							Doxypress controll file for documentation
│   ├── html									Doxypress generarted docs
│   ├── Human_Presence_User_Manual_V1.7.pdf		Later manual from Seed
│   ├── Module_API.html							Html export of Module API.ods
│   ├── Module API.ods							My modified version of the manual
│   ├── Module API.pdf							PDF export of same
│   ├── MR24FDB1_User_manual_v1.0.pdf			Eartlier version of manual
│   ├── pinout.png								Pinout of module
│   ├── protocol.md								Doxpress controll for including dia files
│   └── Protocol.pdf							
├── example
│   ├── platformio.ini							Platformio controll file for example
│   └── src
│       └── main.cpp							Example code
├── library.json								Platformio library manifest
├── LICENSE
├── mr24fdb1.cpp								Driver clode
├── mr24fdb1.h									Class definition
└── README.md									This file
```
