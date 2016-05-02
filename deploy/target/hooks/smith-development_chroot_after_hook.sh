# DIR is the deploy directory on the build system
# tempdir is the root directory of the target filesystem

echo 'Log: in smith-development chroot after hook script'

echo 'Log: removing common chroot functions file from root filesystem directory'
# Remove file copied in before hook
rm -fv "${tempdir}/common_chroot_functions.sh"

echo 'Log: restoring resolv.conf'
# Put the original back
mv -v "${tempdir}/etc/resolv.conf.original" "${tempdir}/etc/resolv.conf"

# Put our modified (I2C0 set to run at 100 kHz) device tree binary in place
echo 'Log: copying modified device tree binary'
_kernel_version=$(echo "${kernel_pkg}" | cut -c13-)
cp -v "${DIR}/setup/boot/dtbs/${_kernel_version}/am335x-boneblack.dtb" "${tempdir}/boot/dtbs/${_kernel_version}/"
