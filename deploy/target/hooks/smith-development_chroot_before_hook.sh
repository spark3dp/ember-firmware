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

echo 'Log: copying resources'
# Copy resource directory so it is available for use in the chroot jail
cp -rv "${DIR}/target/resources" "${tempdir}"

kernel_package_filename="linux-image-3.8.13-bone63_1${deb_codename}_${deb_arch}.tar.gz"
echo "Log: downloading and extracting ${kernel_package_filename}"
wget "https://s3.amazonaws.com/printer-firmware/linux/${kernel_package_filename}"
tar xf "${kernel_package_filename}" -C "${tempdir}"
rm -v "${kernel_package_filename}"

echo 'Log: downloading kernel package(s)'
wget --directory-prefix="${tempdir}" "https://s3.amazonaws.com/printer-firmware/linux/linux-image-3.8.13-bone71_1${deb_codename}_${deb_arch}.deb"
