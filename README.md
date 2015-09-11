## Ember 3D Printer FirmwareVersion 2.1Copyright © 2015 Autodesk, Inc. All rights reserved.###TrademarksAutodesk, Ember, Spark, and the Autodesk logo are registered trademarks or trademarks of Autodesk, Inc., and/or its subsidiaries and/or affiliates.All other brand names, product names or trademarks belong to their respective holders.###PatentsIncludes patents pending.
###Third-Party Software Credits and AttributionsSee the /usr/share/doc and /usr/share/common-licenses folders within Ember itself and the [Ember Firmware Open Source License Disclosure](https://s3.amazonaws.com/printer-firmware/OpenSourceLicenseDisclosure.pdf) for the complete list of third-party copyright notices and licenses.

###Repository Overview
Folder  | Contents
------------- | -------------
/docs  | Doumentation of the firmware architecture and the APIs it exposes to web and local network clients. 
/C++ | The component (smith), that drives the printing process and responds to user input.
/ruby | The components (smith-server and smith-client) that communicate with web and local network applications. Also includes functionality for configuring network connectivity.
/AVR/MotorController | AVR firmware for controlling the motors that move the build head and resin tray, basd on commands from smith.
/AVR/FrontPanel | AVR firmware for controlling the OLED display and LED ring in the front panel.
/AVR/twiboot | The bootloader that loads the AVR firmware via I2C.
/Utilities | Utility for sending commands to motor controller and for controlling AVR bootloader.
/deploy | Scripts for packaging and deploying the firmware and building development systems.
/infrastructure | Components to assist with automated builds.

###Development System Image
An image of the SD card we use for development can be downloaded from [here](http://printer-firmware.s3-website-us-east-1.amazonaws.com/development_image).  To use it, flash it to a micro SD card (2GB minimum), boot a BeagleBone Black or an Ember using the SD card, then ssh in (root@192.168.7.2 via network over USB) and run:

```
resize-rootfs
reboot
```

