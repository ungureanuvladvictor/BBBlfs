#!/bin/bash

scriptPath=$(dirname $(readlink -f $BASH_SOURCE))
. $scriptPath/common.sh

install_required_tools

#current directory
current=$1

echo Building the kernel
kernel_source=/tmp/bb-kernel

git clone https://github.com/beagleboard/kernel $kernel_source
pushd $kernel_source
  git checkout 3.14
  ./patch.sh
  cp configs/beaglebone kernel/arch/arm/configs/beaglebone_defconfig
  wget -c http://arago-project.org/git/projects/?p=am33x-cm3.git\;a=blob_plain\;f=bin/am335x-pm-firmware.bin\;hb=HEAD -O kernel/firmware/am335x-pm-firmware.bin
  pushd kernel
    make ARCH=arm CROSS_COMPILE=$CROSS_COMPILE beaglebone_defconfig -j4
    make ARCH=arm CROSS_COMPILE=$CROSS_COMPILE zImage dtbs modules -j4
  popd
popd

echo Building the ramdisk based of BusyBox
ramdisk_source=/tmp/initramfs
mkdir $ramdisk_source
mkdir -p $ramdisk_source/{bin,sbin,etc,proc,sys}

pushd $ramdisk_source
  wget -O init https://raw.githubusercontent.com/ungureanuvladvictor/BBBlfs/master/tools/init
  chmod +x init
popd

busybox_source=/tmp/busybox

git clone git://git.busybox.net/busybox $busybox_source
pushd $busybox_source
  make ARCH=arm CROSS_COMPILE=$CROSS_COMPILE defconfig
  make ARCH=arm CROSS_COMPILE=$CROSS_COMPILE menuconfig
  make ARCH=arm CROSS_COMPILE=$CROSS_COMPILE -j4
  make ARCH=arm CROSS_COMPILE=$CROSS_COMPILE install CONFIG_PREFIX=$ramdisk_source
  pushd $kernel_source/kernel
    make ARCH=arm CROSS_COMPILE=$CROSS_COMPILE modules_install INSTALL_MOD_PATH=$ramdisk_source
  popd
popd

echo Package everything up
maker_source=/tmp/maker
mkdir $maker_source

pushd $ramdisk_source
  find . | cpio -H newc -o > $current/initramfs.cpio
popd 
cat $current/initramfs.cpio | gzip > $current/initramfs.gz
rm $current/initramfs.cpio
mv $current/initramfs.gz $maker_source/ramdisk.cpio.gz

pushd $maker_source
  wget -O maker.its https://raw.githubusercontent.com/ungureanuvladvictor/BBBlfs/master/tools/maker.its
  cp $kernel_source/kernel/arch/arm/boot/zImage .
  cp $kernel_source/kernel/arch/arm/boot/dts/am335x-boneblack.dtb .
  mkimage -f maker.its FIT
  cp -p FIT $current/fit
popd
