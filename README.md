
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

in the test directory is a platformio project directory that can be used as a starting point.
copy the test directory to somewehere
copy  the this (MR24FDB1) directory into test/lib
clone down the debug library (see below on dependencies) to test/lib as well
in the test direcoty run platformio run build and it should build

# Known or rather uknown stuff

Not all report functions and data are documented in the documentation (or acording to me correctly named/documented):

1. Proactive report (0x04) / Radar Information (0x03) /Aproaching Away state (0x07)
  Should accoirding to the manual report 3 bytes where the 2 firs are allways 0x01 0x01 and the the
  third should be on of 0x1 [None], 0x02 [Close], 0x03 [Stay Away].
  But i have seens bothe 0x04 and 0x05 reported. I have not yet figured out exactly when they are reported
  but it seems connected to "vigours" movment or perhaps transversal..

2. Proactive report (0x04) / Other Information (0x03) / 0x09 (upgrade packet ? ) 
  Vill once in a wile be reported , this is not documented as an report [Proactive report] function.
  It does exist as a Responce [Passive Report] but then in conjunction with ota upgrade

# Dependencies

It uses my debug library: github.com:Kilill/debug.git
this can be stripped out by removing any lines that as FAIL/ERROR/WARNING/INFO/DBG in them
them...
 
# Docs


The doc direcotry contains the modified documentation and class description

# File structure
```
├── doc
│   ├── html					doxypress generated Html files of documentation
│   ├── Module_API.csv			CSV dump of api doc
│   ├── Module_API.html			simple html export
│   ├── Module API.ods			Api documentation in Libre Office Calc
│   ├── Module API.pdf			PDF Export of same
│   └── pinout.png				Pinout of module
├── include
│   └── README
├── lib
│   ├── debug					Debug printing routines
│   │   ├── dbgLevels.h			Debug level bit maps. Include before debug.h
│   │   ├── debug.cpp			Debug printing utilities
│   │   └── debug.h				Debug macro definitions
│   └── mr24fdb1				Driver directory
│       ├── decodeError.cpp		Decodes exceptions thrown
│       ├── library.json		Library description for platformio
│       ├── mr24fdb1.cpp		Driver class code
│       └── mr24fdb1.h			Driver class definition	
├── platformio.ini				Platformio config file
├── README						This file
├── src
│   ├── main.cpp				Example main file
│   └── README
└── test						Tests
    └── README
```
