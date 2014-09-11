- Get u-boot source:
    git clone git://git.denx.de/u-boot.git
    cd u-boot/
    git checkout v2014.07 -b tmp

- Apply uEnv patch from BeagleBone community:
    wget -c https://raw.githubusercontent.com/eewiki/u-boot-patches/master/v2014.07/0001-am335x_evm-uEnv.txt-bootz-n-fixes.patch
    patch -p1 < 0001-am335x_evm-uEnv.txt-bootz-n-fixes.patch

- Apply BeagleBone Black hardcode patch:
    cp Smith/Firmware/deploy/u-boot/hardcode-bbb.patch .
    patch -p1 < hardcode-bbb.patch 

- Build:
    make ARCH=arm CROSS_COMPILE=${CC} distclean
    make ARCH=arm CROSS_COMPILE=${CC} am335x_evm_config
    make ARCH=arm CROSS_COMPILE=${CC}

The resulting u-boot.img and MLO files can be copied to a boot partition

Reference: https://eewiki.net/display/linuxonarm/BeagleBone+Black#BeagleBoneBlack-Bootloader:U-Boot
