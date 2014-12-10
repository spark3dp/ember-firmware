Main Page {#mainpage}
=========

Autodesk 3D Printer Control Panel firmware (rev A.3)

The revA.3 Control Panel contains two tactile buttons (active low, software
debounced) and 21 led ring and a colour OLED.  


## Documentation Map

* the firmware entry points (setup and loop) are in \ref firmware.ino
  "firmware.ino"
* see the \ref interface "interface documentation" for the interface API
* see the \ref Ring "Ring class" for led red functions including animations
* see the \ref Oled "Oled class" for Oled function (current firmware uses Adafruit
  libraries)
* see the \ref I2CInterface "I2CInterface class" for interface functions
* hardware mappings are in \ref hardware-revA3.h "hardware-revA3.h"
* variables can be defined in \ref variables.h "variables.h"

