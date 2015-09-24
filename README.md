## Ember 3D Printer Firmware
Version 2.1

Copyright © 2015 Autodesk, Inc. All rights reserved.

###Repository Overview
The [Autodesk Ember](https://ember.autodesk.com/) 3D printer uses the firmware here to control its operation and communicate with applications.  The components in the C++ and ruby folders run on the main (Sitara) processor under Linux and the others run on two satelite AVRs that control the motors and front panel.

Folder  | Contents
------------- | -------------
/docs  | Documentation of the firmware architecture and the APIs it exposes to web and local network clients. 
/C++ | Component (smith), that drives the printing process and responds to user input.  Includes NetBeans project for rebuilding.
/ruby | Components (smith-server and smith-client) that communicate with web and local network applications.  Also includes functionality for configuring network connectivity.
/AVR/MotorController | AVR firmware for controlling the motors that move the build head and resin tray, based on commands from smith.
/AVR/FrontPanel | AVR firmware for controlling the OLED display and LED ring in the front panel.
/AVR/twiboot | Bootloader that loads AVR firmware via I2C.
/Utilities | Utility for sending commands to motor controller and for controlling AVR bootloader.
/deploy | Scripts for packaging and deploying the firmware and building development systems.
/infrastructure | Components to assist with automated builds.

###Contributing to Ember Firmware
We welcome contributions to the Ember firmware.  You can do so through GitHub by forking the repository and sending a pull request.

Active development should occur on a feature branch. Changes will be merged into the master branch by our project committers ahead of release.

When submitting code, please make every effort to follow existing conventions and style in order to keep the code as readable as possible. Our project has adopted the style guidelines for C++ code given in the /docs folder.

When submitting a pull request, please ensure you include enough information so the reviewer can understand the changes being made and the motivations.

For comments, feature requests, or questions, please create a GitHub issue.
 

###Development Image
An image of the SD card we use for development can be downloaded from [here](http://printer-firmware.s3-website-us-east-1.amazonaws.com/development_image).  To use it, flash it to a micro SD card (2GB minimum, must write image directly to SD card using [Win32DiskImager](http://sourceforge.net/projects/win32diskimager/), ```dd```, or similar; copying the image to an existing partition will not work), boot a BeagleBone Black or an Ember using the SD card, then ssh in (root@192.168.7.2 via network over USB) and run:

```
resize-rootfs
reboot
```

The currently available development image includes the currently released version of the firmware (as indicated in the ```README``` in the ```master``` branch). By default, the firmware applications will not start when the printer boots. If you want the firmware applications to start when the printer boots, as is the case with the production firmware releases, ssh in and run:

```
systemctl enable smith.service
systemctl enable smith-client.service
```

The web client and main firmware programs can also run from the command line using these commands:

```
smith
smith-client
```

The development image provides the same functionality as the release firmware with the following exceptions:

- The file system is writable. This allows customization through rebuilding the firmware, custom scripts, etc.
- The firmware included in the image cannot be upgraded through the upgrade firmware functionality provided by the printer's local web interface or emberprinter.com. The easiest way to upgrade the firmware on a development image is to use the currently available development image. 

###Trademarks
Autodesk, Ember, Spark, and the Autodesk logo are registered trademarks or trademarks of Autodesk, Inc., and/or its subsidiaries and/or affiliates.
All other brand names, product names or trademarks belong to their respective holders.

###Patents
Includes patents pending.

###Third-Party Software Credits and Attributions
See the /usr/share/doc and /usr/share/common-licenses folders within Ember itself 
and the [Ember Firmware Open Source License Disclosure](https://s3.amazonaws.com/printer-firmware/OpenSourceLicenseDisclosure.pdf) for the complete list of third-party copyright notices and licenses.

###Disclaimer
THIS FIRMWARE IS PROVIDED BY THE COPYRIGHT AND TRADEMARK HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT AND/OR TRADEMARK HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION), INCLUDING DAMAGES OR CONSEQUENCES ASSOCIATED WITH USE OF THE FIRMWARE — WHETHER MODIFIED OR NOT — IN TANGIBLE DEVICES OR PRODUCTS, HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY IN CONNECTION WITH THE FIRMWARE OR THE USE OR OTHER DEALINGS IN THE FIRMWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

