#!/usr/bin/env bash

#Original name: bbb-debian.sh
#Original script posted on the comments by calzon65 from armhf.com

set -e

# This script is used to create a new (clean) install of Debian ARM HF (armhf.com) to the
# BeagleBone Black's onboard eMMC storage. This script assumes you have booted the BeagleBone
# Black using a microSD card, you are running as root, and you want to install Debian to the
# onboard eMMC storage.

# If your microSD is running a different flavor of Linux (e.g., ArchLinux ARM) it also assumes
# you have mkfs (vfat & ext4) and wget installed. If further assumes wget and tar work as in
# Debian, which is not always true.

drive=$1
rootfs=$2
BOOTDEV=/dev/${drive}1
ROOTDEV=/dev/${drive}2
WORKINGDIR=/tmp
BOOTTMP=$WORKINGDIR/tmpboot
ROOTTMP=$WORKINGDIR/tmprootfs

SOURCE=http://s3.armhf.com/dist/bone/
BOOTLOADER=bone-uboot.tar.xz
if [ "${rootfs}" == "ubuntu" ]
then
  LINUX=ubuntu-trusty-14.04-rootfs-3.14.4.1-bone-armhf.com.tar.xz
else
  LINUX=debian-wheezy-7.5-rootfs-3.14.4.1-bone-armhf.com.tar.xz
fi

test=$(df | grep $BOOTDEV | awk '{ print $6 }')
if [ -n "${test}" ]
then
  umount $test
fi
test=$(df | grep $ROOTDEV | awk '{ print $6 }')
if [ -n "${test}" ]
then
  umount $test
fi

clear
echo "If you are booted from a microSD card, this script will"
echo "copy Debian (ARM HF) (www.armfh.com) to the BBB's eMMC."
echo
echo "If you are booted from the eMMC AND have a microSD card inserted,"
echo "this script may inadvertently copy to the microSD card."
echo
echo "So ensure you are booted from the microSD card."
echo
echo "u-boot files will be copied to" $BOOTDEV
echo "rootfs files will be copied to" $ROOTDEV
echo
echo "WARNING: ALL DATA on" $BOOTDEV "and" $ROOTDEV "WILL BE ERASED!"

while true
do
  echo 
  read -p "Do you want to continue (answer yes or no)? " ANSWER
  case $ANSWER in
    yes) break
      ;;
    no) exit
      ;;
    *) echo
       echo "ERROR: Answer [yes|no]"
      ;;
  esac
done

if [ ! -w $BOOTDEV -o ! -w $ROOTDEV ]
then
  echo
  echo "ERROR:" $BOOTDEV "and/or" $ROOTDEV "not present or writable."
  echo " Ensure you are booted from the microSD card and running as root."
  echo
  exit
fi

if [ ! -d $WORKINGDIR ]
then
  echo "Working Directory ($WORKINGDIR) Missing"
  echo "Script Terminating"
  exit
fi

if [ ! -r $WORKINGDIR/$BOOTLOADER ]
then
  wget $SOURCE$BOOTLOADER -O $WORKINGDIR/$BOOTLOADER
fi

if [ ! -r $WORKINGDIR/$LINUX ]
then
  wget $SOURCE$LINUX -O $WORKINGDIR/$LINUX
fi

if [ ! -d $ROOTTMP ]
then
  mkdir -p $ROOTTMP
fi
if [ ! -d $BOOTTMP ]
then
  mkdir -p $BOOTTMP
fi

mkfs.vfat -F 16 -n "boot" $BOOTDEV
sleep 1
mkfs.ext4 -L "rootfs" $ROOTDEV
sleep 1

mount $BOOTDEV $BOOTTMP
mount $ROOTDEV $ROOTTMP

tar xJvf $WORKINGDIR/$BOOTLOADER -C $BOOTTMP
sleep 1
tar xJvf $WORKINGDIR/$LINUX -C $ROOTTMP
sleep 1

echo "Synching"
sync
sleep 1

umount $BOOTTMP
umount $ROOTTMP

echo
echo "Upload Complete"

while true
do
  echo 
  read -p "Do you want to remove the downloaded files and temp directories (answer yes or no)? " ANSWER
  case $ANSWER in
    yes) rm $WORKINGDIR/$BOOTLOADER
         rm $WORKINGDIR/$LINUX
         rmdir $BOOTTMP
         rmdir $ROOTTMP
         break
      ;;
    no) break
      ;;
    *) echo
       echo "ERROR: Answer [yes|no]"
      ;;
  esac
done
