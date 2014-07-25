BBBlfs
======

Beagle Bone Black Linux Flash System

This project provides a way to flash a BeagleBone Black via USB from a Linux machine. The project is developed during Google Summer of Code '13. 

#Build
- sudo apt-get install libusb-1.0-0-dev
- make dirs
- make

Press the S2 button on the BeagleBone Black  and apply power to the board. The board should start now into USB boot mode. 

Connect the board to the host PC. The kernel should now identify your board as an RNDIS interface. Be sure you do not have any BOOTP servers on your network.

Go to bin/ and execute flash_script.sh. It needs the flashing image as argument to be provided.

For now only .xz compressed images are supported.

#Usage
- sudo ./flash_script.sh image.xz

If there are bugs please feel free to contact me.


#Contact
vvu@vdev.ro

vvu on #beagle, #beagle-gsoc

