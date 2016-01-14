# DIR is the deploy directory on the build system
# tempdir is the root directory of the target filesystem

echo 'Log: in smith-development chroot before hook script'

echo 'Log: installing configuration files'
# Copy common config files
cp -rv "${DIR}/target/config/smith-common/". "${tempdir}"
# Copy development specific config files
cp -rv "${DIR}/target/config/smith-development/". "${tempdir}"

echo 'Log: copying common chroot functions file to root filesystem directory'
# Copy the common chroot functions so they can be used in the chroot
cp -v "${DIR}/target/scripts/common_chroot_functions.sh" "${tempdir}"

echo 'Log: replacing resolv.conf in chroot jail with copy from host'
# resolv.conf doesn't get updated properly in the chroot so just copy the contents
# of the resolv.conf file on the host into the target filesystem
# This allows DNS lookups (required for apt-get) to work in the chroot script
# This gets undone in the after hook script
mv -v "${tempdir}/etc/resolv.conf" "${tempdir}/etc/resolv.conf.original"
cat "/etc/resolv.conf" > "${tempdir}/etc/resolv.conf"

# Put our modified (I2C0 set to run at 100 kHz) device tree binary in place
_kernel_version=$(echo "${kernel_pkg}" | cut -c13-)
cp -v "${DIR}/setup/boot/dtbs/${_kernel_version}/am335x-boneblack.dtb" "${tempdir}/boot/dtbs/${_kernel_version}/"
