BBBabb
======

Beagle Bone Black Android Based Boot System

This project provides a way to boot a BB Black or White(same SoC) via USB from a Linux machine. The project is developed during Google Summer of Code '13. 
For more information: http://www.google-melange.com/gsoc/proposal/review/google/gsoc2013/ungureanuvladvictor/1001

#Build
- sudo apt-get install libusb-1.0-0-dev
- make

The binary will be located in the bin folder. The syntax is ./boot emmc / sdcard image.

Press the S2 button on the BBB/BBW and apply power to the board. The board should start now into USB boot mode. 

Connect the board to the host PC. The kernel should now identify your board as an RNDIS interface. Be sure you do not have any BOOTP servers on your network.

Run the binary and profit! The binary sends the archived image to the board. While it is sending the image all 4 LEDs will be flashing. After the program quits you can disconnect the USB cable from the Beagle. Once the flashing is done only 2 LEDs will be flashing. Reboot the board and will start with your new OS.

If there are bugs please feel free to contact me.


#Contact
ungureanuvladvictor@gmail.com
vvu or vvu|Mobile on #beagle, #beagle-gsoc

