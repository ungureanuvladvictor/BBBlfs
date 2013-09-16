BBBabb
======

Beagle Bone Black Android Based Boot System

This project provides a way to boot a BB Black or White(same SoC) via USB from a Linux machine. The project is developed during Google Summer of Code '13. 
For more information: http://www.google-melange.com/gsoc/proposal/review/google/gsoc2013/ungureanuvladvictor/1001

#Build
To build the project libusb-1.0-0-dev is needed on the system. A makefile is provided.
The binary will be located in the bin folder. The format of the command is ./boot emmc / sdcard image.
To flash the eMMC choose emmc / sdcard otherwise. The image needs to be either an .xz or .zip image. More formats will be supported in the future.

#Contact
ungureanuvladvictor@gmail.com
vvu or vvu|Mobile on #beagle, #beagle-gsoc

