#!/bin/bash

scriptPath=$(dirname $(readlink -f $BASH_SOURCE))
. $scriptPath/common.sh

install_required_tools

#current directory
current=$1

echo Building custom U-Boot
git clone git://git.denx.de/u-boot.git /tmp/u-boot
pushd /tmp/u-boot
  git reset --hard
  git checkout 524123a70761110c5cf3ccc5f52f6d4da071b959
  wget -c https://raw.githubusercontent.com/ungureanuvladvictor/BBBlfs/master/tools/USB_FLash.patch
  patch -p1 < USB_FLash.patch
  make ARCH=arm CROSS_COMPILE=$CROSS_COMPILE distclean
  make ARCH=arm CROSS_COMPILE=$CROSS_COMPILE am335x_evm_usbspl_config
  make ARCH=arm CROSS_COMPILE=$CROSS_COMPILE
  cp -p u-boot.img $current/uboot
  cp -p spl/u-boot-spl.bin $current/spl
popd
