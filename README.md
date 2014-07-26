BBBlfs
======

Beagle Bone Black Linux Flash System

This project provides a way to flash a BeagleBone Black via USB from a Linux machine. The project is developed during Google Summer of Code '13. 

#Build
- sudo apt-get install libusb-1.0-0-dev
- make


#Usage

Press the S2 button on the BeagleBone Black and apply power to the board. The board should start now into USB boot mode. 

Connect the board to the host PC. The kernel should now identify your board as an RNDIS interface. Be sure you do not have any BOOTP servers on your network.

Go to bin/ and execute flash_script.sh. It needs the flashing image as argument to be provided.

For now only .xz compressed images are supported.

- sudo ./flash_script.sh image.xz

If there are bugs please feel free to contact me.

#How to build the binary blobs
The full system works as follow:

* The AM335x ROM when in USB boot mode exposes a RNDIS interface to the PC and waits to TFTP a binary blob that it executes. That binary blob is the SPL
* Once the SPL runs it applies the same proceure as the ROM and TFTPs U-Boot
* Again U-Boot applies the same thinking and TFTPs a FIT(flatten image tree) which includes a Kernel, Ramdisk and the DTB
* U-Boot runs this FIT and boots the Kernel
* When the kernel starts the init script exports the eMMC using the g_mass_storage kernel module as an USB stick to the Linux so it can be flashed


* ## Building U-Boot for USB booting
    * Grab the latest U-Boot sources from [git://git.denx.de/u-boot.git](git://git.denx.de/u-boot.git)
    * Install your favourite cross-compiler, I am using arm-linux-gnueabi-
    * Apply this patch to U-Boot sources [https://raw.githubusercontent.com/ungureanuvladvictor/BBBlfs/master/tools/USB_FLash.patch](https://raw.githubusercontent.com/ungureanuvladvictor/BBBlfs/master/tools/USB_FLash.patch )
    	* make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- am335x_evm_usbspl
    * Now you have u-boot.img which is the uboot binary and spl/u-boot-spl.bin which is the spl binary

* ## Building the Kernel
    * Grab the latest from [https://github.com/beagleboard/kernel](https://github.com/beagleboard/kernel)
    	* git checkout 3.14
    	* ./patch.sh
    	* cp configs/beaglebone kernel/arch/arm/configs/beaglebone_defconfig
    	* wget http://arago-project.org/git/projects/?p=am33x-cm3.git\;a=blob_plain\;f=bin/am335x-pm-firmware.bin\;hb=HEAD -O kernel/firmware/am335x-pm-firmware.bin
    	* cd kernel
    	* make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- beaglebone_defconfig -j4
    	* make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- zImage dtb modules -j4
    * After compilation you have in arch/arm/boot/ the zImage

* ## Building the ramdisk
    * Our initramfs will be built around BusyBox. First we create the basic folder structure.
    	* mkdir initramfs
    	* mkdir -p initramfs/{bin,sbin,etc,proc,sys}
    	* cd initramfs
    	* wget -O init [https://raw.githubusercontent.com/ungureanuvladvictor/BBBlfs/master/tools/init](https://raw.githubusercontent.com/ungureanuvladvictor/BBBlfs/master/tools/init)
    	* chmod +x init
    * Now we need to cross-compile BusyBox for our ARM architecture
    	* git clone git://git.busybox.net/busybox
    	* cd busybox
    	* make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- defconfig
    	* make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- menuconfig
    * Now here we need to compile busybox as a static binary: Busybox Settings --> Build Options --> Build Busybox as a static binary (no shared libs)  -  Enable this option by pressing "Y"
    	* make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- -j4
    	* make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- install CONFIG_PREFIX=/path/to/initramfs
    * Now we need to install the kernel modules in our initramfs
    	* cd /path/to/kernel/sources
    	* make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- modules_install INSTALL_MOD_PATH=/path/to/initramfs

* ## Packing things up
    * Now we need to put our initramfs in a .gz archive that the kernel knows how to process
    	* mkdir maker
    	* cd /path/to/initramfs
    	* find . | cpio -H newc -o > ../initramfs.cpio
    	* cd .. && cat initramfs.cpio | gzip > initramfs.gz
    	* mv initramfs.gz /path/to/maker/folder/ramdisk.cpio.gz
    * Now we need to pack all things in a FIT image. In order to do so we need some additional packages installed, namely the mkimage and dtc compiler.
    	* sudo apt-get update
    	* sudo apt-get install u-boot-tools device-tree-compiler
    	* cd /path/to/maker/folder
    	* wget -O maker.its [https://raw.githubusercontent.com/ungureanuvladvictor/BBBlfs/master/tools/maker.its](https://raw.githubusercontent.com/ungureanuvladvictor/BBBlfs/master/tools/maker.its)
    	* cp /path/to/kernel/arch/arm/boot/zImage .
    	* cp /path/to/kernel/arch/arm/boot/dts/am335x-boneblack.dtb .
    	* mkimage -f maker.its FIT
    * At this point we have all things put into place. You need to copy the binary blobs in the bin/ folder and run flash_script.sh

#Contact
vvu@vdev.ro

vvu on #beagle, #beagle-gsoc
