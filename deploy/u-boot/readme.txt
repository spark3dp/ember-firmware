- Get u-boot source:
    git clone git://git.denx.de/u-boot.git
    cd u-boot/
    git checkout v2015.10 -b tmp

- Apply uEnv patch from BeagleBone community:
    wget -c https://rcn-ee.com/repos/git/u-boot-patches/v2015.10/0001-am335x_evm-uEnv.txt-bootz-n-fixes.patch
    patch -p1 < 0001-am335x_evm-uEnv.txt-bootz-n-fixes.patch

- Apply BeagleBone Black hardcode patch:
    cp Firmware/deploy/u-boot/0001-hardcode-BeagleBone-Black-rev-B.patch .
    patch -p1 < 0001-hardcode-BeagleBone-Black-rev-B.patch

- Build:
    make ARCH=arm CROSS_COMPILE=${CC} distclean
    make ARCH=arm CROSS_COMPILE=${CC} am335x_evm_config
    make ARCH=arm CROSS_COMPILE=${CC}

The resulting u-boot.img and MLO files can be copied to a boot partition or an image.

Reference: https://eewiki.net/display/linuxonarm/BeagleBone+Black#BeagleBoneBlack-Bootloader:U-Boot
