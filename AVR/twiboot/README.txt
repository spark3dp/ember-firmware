This code was forked from twiboot, which can be found here: https://github.com/orempel/twiboot and here: http://git.kopf-tisch.de/?p=twiboot

The code for the Linux utility used to interface with the bootloader over I2C can be found here: https://github.com/orempel/multiboot_tool
The Linux utility is used without any modifications

The SHA1 ID of the last git commit before the code was forked is bc5d45359aa9370e6665c5c03eb5c3fff13db23e

Changes made to the original twiboot code:
  - Add support for atmega328p
  - Add command to read crc16
  - Compute crc16 on startup
  - Disable LED support
  - Disable automatic switching to application on boot
