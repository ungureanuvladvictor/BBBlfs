echo
wget -c https://releases.linaro.org/14.04/components/toolchain/binaries/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux.tar.xz
tar xf gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux.tar.xz
export CROSS_COMPILE=`pwd`/gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux/bin/arm-linux-gnueabihf-
echo The cross compiler is set to $CROSS_COMPILE
echo

unset deb_pkgs

function check_dpkg() {
  LC_ALL=C dpkg --list | awk '{print $2}' | grep "^${pkg}" >/dev/null || deb_pkgs="${deb_pkgs}${pkg} "
}

function aptget_update_install() {
  if [ "${deb_pkgs}" ]; then
    sudo apt-get update
    sudo apt-get -y install ${deb_pkgs}
    sudo apt-get clean
  fi
}

function install_required_tools() {
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

  aptget_update_install
}
