## Ember 3D Printer Firmware
Version 3.1

Copyright © 2016 Autodesk, Inc. All rights reserved.

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
An image of the MicroSD card we use for development can be downloaded from [here](http://printer-firmware.s3-website-us-east-1.amazonaws.com/development_image). The development image always has latest released version of the firmware (as indicated in the ```README``` in the ```master``` branch). To use the development image:
1. Flash it to a MicroSD card (2GB minimum) using either [Win32DiskImager](http://sourceforge.net/projects/win32diskimager/), ```dd```, [Apple Pi Baker](http://www.tweaking4all.com/?wpfb_dl=94)(Mac), or similar. The image must written directly to MicroSD card using the above methods; copying the image to an existing partition will not work.
2. Insert the MicroSD card into an Ember or BeagleBone Black, plug in power, and connect it to your computer via USB.
3. Connect to the device using [SSH](https://support.ember.autodesk.com/hc/en-us/articles/218519168-SSH-Into-Ember) (root@192.168.7.2)
4. Run the following commands:

```
resize-rootfs
reboot
```

By default, the firmware applications (the Ember 3D printing engine and its link to emberprinter.com) will NOT start when the printer boots. You can start them manually as background processes, set them to start automatically when the printer boots, or start them manually as foreground processes.

To start them manually as background processes, run the following commands:

```
systemctl start smith-client.service
systemctl start smith.service
```

To set them to start automatically when the printer boots, as is the case with production firmware releases, run the same commands but substitute `enable` in place of `start` (note that this doesn't take effect until you reboot):

```
systemctl enable smith.service
systemctl enable smith-client.service
```

To start them manually in the foreground, which enables you to enter commands directly without calling `echo [command] > /tmp/CommandPipe`), run the following commands in separate windows:

```
smith-client
smith
```
If you just want to see their output without enter commands, you can just run `tail -f /var/log/syslog` instead.

The development image provides the same functionality as the release firmware with the following exceptions:
- The file system is writable. This allows customization through [rebuilding the firmware](https://github.com/spark3dp/ember-firmware/#building-smith), custom scripts, [network configuration](https://support.ember.autodesk.com/hc/en-us/articles/227350528-Give-Ember-a-Static-IP-Address-for-a-Wired-Network), etc.
- The development image firmware cannot be upgraded via the Upgrade Firmware functionality provided by the printer's local web interface or emberprinter.com. The easiest way to upgrade the firmware on a development image is to re-download the development image, which is always updated with the latest firmware.

###Building Smith
Smith is Ember's 3D printing engine, which runs within the larger firmware system. If your goal in modifying Ember's firmware is to change how Ember prints (or even to use its projector, motors, and sensors to do something else entirely), you only need to build smith, not the entire firmware system. Building smith is an order of magnitude faster, and does not require internet access. The only caveat is that you'll always need to be running off a development image on a MicroSD card, whereas rebuilding the entire firmware system and installing it allows you to boot without a MicroSD card, which also means that it can be easily installed on multiple printers.

We use CMake as the build system for the smith. To build smith, first copy the entire ```C++``` directory to an Ember or BeagleBone Black booted from a development MicroSD card. Then execute the following commands:

```
cd C++
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
make install
```

Once it finishes, type:
```
systemctl start smith-client.service
systemctl start smith.service
```
to start the Ember 3D printing engine.

We also have a NetBeans project in the source tree, which is a better choice for continuous development. To use it, add your Ember or BeagleBone black as a remote build host in NetBeans and attempt, through NetBeans, to build the project on the remote host. When building for the first time, NetBeans will complain that the ```build``` directory does not exist. Copy the full path listed in the NetBeans error message, SSH into the build host, create the build directory using ```mkdir``` and the full path to the build directory copied from NetBeans. ```cd``` to the newly created build directory and run the CMake command listed above. Then trigger a build through NetBeans.

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
