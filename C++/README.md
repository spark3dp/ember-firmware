#Building
We use CMake as the build system for the C++ code. To build the C++ code, first copy the entire ```C++``` directory to an Ember or BeagleBone Black booted from a development SD card. Then execute the following commands:

```
cd C++
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
``` 

We also have a NetBeans project in the source tree. To use, add your Ember or BeagleBone black as a remote build host in NetBeans and attempt, through NetBeans, to build the project on the remote host. When building for the first time, NetBeans will complain that the ```build``` directory does not exist. Copy the full path listed in the NetBeans error message, SSH into the build host, create the build directory using ```mkdir``` and the full path to the build directory copied from NetBeans. ```cd``` to the newly created build directory and run the CMake command listed above. Then trigger a build through NetBeans.