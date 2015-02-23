#!/bin/bash

show_help() {
cat << EOF
Usage: ${0##*/} [-ufh]
Compiles all the files for BBBlfs.

	-h 			display this help and exit
	-f 			compiles just the fit image
	-u 			compiles just u-boot
EOF
}

if [ $# -eq 0 ]; then
	show_help
	exit 1
fi
unset deb_pkgs

check_dpkg() {
	LC_ALL=C dpkg --list | awk '{print $2}' | grep "^${pkg}" >/dev/null || deb_pkgs="${deb_pkgs}${pkg} "
}

function aptget_update_install () {
	if [ "${deb_pkgs}" ]; then
		sudo apt-get update
		sudo apt-get -y install ${deb_pkgs}
		sudo apt-get clean
	fi
}

function install_required_tools () {
	pkg="git"
	check_dpkg
	pkg="pkg-config"
	check_dpkg
	pkg="libc6:i386"
	check_dpkg
	pkg="libstdc++6:i386"
	check_dpkg
	pkg="libncurses5:i386"
	check_dpkg
	pkg="device-tree-compiler"
	check_dpkg
	pkg="lzma"
	check_dpkg
	pkg="lzop"
	check_dpkg
	pkg="u-boot-tools"
	check_dpkg
	pkg="libncurses5-dev:i386"
	check_dpkg
	pkg="zlib1g:i386"
	check_dpkg

	aptget_update_install
}

check_cc() {
	if which arm-linux-gnueabihf-gcc > /dev/null; then
		export CROSS_COMPILE=arm-linux-gnueabihf-
	else
		wget -c https://releases.linaro.org/14.04/components/toolchain/binaries/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux.tar.xz
		tar xf gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux.tar.xz
		export CROSS_COMPILE=`pwd`/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux/bin/arm-linux-gnueabihf-
	fi
}

compile_u-boot() {
	scriptPath=$(dirname $(readlink -f $BASH_SOURCE))
	mkdir -p ${scriptPath}/bintest
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
	cp -p u-boot.img ${scriptPath}/bintest/uboot
	cp -p spl/u-boot-spl.bin ${scriptPath}/bintest/spl
	popd
}

compile_fit() {
	scriptPath=$(dirname $(readlink -f $BASH_SOURCE))
	mkdir -p ${scriptPath}/bintest
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
		find . | cpio -H newc -o > ${scriptPath}/bintest/initramfs.cpio
	popd
	cat ${scriptPath}/bintest/initramfs.cpio | gzip > ${scriptPath}/bintest/initramfs.gz
	rm ${scriptPath}/bintest/initramfs.cpio
	mv ${scriptPath}/bintest/initramfs.gz $maker_source/ramdisk.cpio.gz

	pushd $maker_source
		wget -O maker.its https://raw.githubusercontent.com/ungureanuvladvictor/BBBlfs/master/tools/maker.its
		cp $kernel_source/kernel/arch/arm/boot/zImage .
		cp $kernel_source/kernel/arch/arm/boot/dts/am335x-boneblack.dtb .
		mkimage -f maker.its FIT
		cp -p FIT ${scriptPath}/bintest/fit
	popd
}

check_cc
install_required_tools

while getopts "ufh" opt; do
	case "$opt" in
		h)
			show_help
			exit 0
			;;
		u)
			compile_u-boot
			;;
		f)
			compile_fit
			;;

		'?')
			show_help >&2
			exit 1
			;;
		*)
			echo "unrecognized option"
			;;
	esac
done
