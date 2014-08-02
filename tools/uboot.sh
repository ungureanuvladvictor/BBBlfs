#!/bin/bash

scriptPath=$(dirname $(readlink -f $BASH_SOURCE))
. $scriptPath/common.sh

install_required_tools

#current directory
current=$1

echo Building custom U-Boot
echo CROSS: $CROSS_CC
git clone git://git.denx.de/u-boot.git /tmp/u-boot
pushd /tmp/u-boot
  git reset --hard
  wget -c https://raw.githubusercontent.com/ungureanuvladvictor/BBBlfs/master/tools/USB_FLash.patch
  patch -p1 < USB_FLash.patch
  make ARCH=arm CROSS_COMPILE=$CROSS_CC am335x_evm_usbspl_defconfig
  make ARCH=arm CROSS_COMPILE=$CROSS_CC
  cp -p u-boot.img $current/uboot
  cp -p spl/u-boot-spl.bin $current/spl
popd
