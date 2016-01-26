# DIR is the deploy directory on the build system
# tempdir is the root directory of the target filesystem

echo 'Log: in smith-release chroot before hook script'

echo 'Log: installing configuration files'
# Copy common config files
cp -rv "${DIR}/target/config/smith-common/". ${tempdir}
# Copy release specific config files
cp -rv "${DIR}/target/config/smith-release/". ${tempdir}

echo 'Log: copying common chroot functions file to root filesystem directory'
# Copy the common chroot functions so they can be used in the chroot
cp -v "${DIR}/target/scripts/common_chroot_functions.sh" "${tempdir}"

kernel_package_filename="linux-image-3.8.13-bone63_1${deb_codename}_${deb_arch}.tar.gz"
echo "Log: downloading and extracting ${kernel_package_filename}"
wget "https://s3.amazonaws.com/printer-firmware/linux/${kernel_package_filename}"
tar xf "${kernel_package_filename}" -C "${tempdir}"
rm -v "${kernel_package_filename}"

echo 'Log: downloading kernel package(s)'
wget --directory-prefix="${tempdir}" "https://s3.amazonaws.com/printer-firmware/linux/linux-image-3.8.13-bone71_1${deb_codename}_${deb_arch}.deb"
