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

echo 'Log: downloading kernel package(s)'
for _kernel_pkg in $kernel_pkg_list; do
  _kernel_pkg_url="https://s3.amazonaws.com/printer-firmware/linux/${_kernel_pkg}_1${deb_codename}_${deb_arch}.deb"
  if ! wget --directory-prefix="${tempdir}" "${_kernel_pkg_url}"; then
    echo "Unable to download ${_kernel_pkg_url}, did you upload the correct kernel package to our S3 server?"
    exit 1
  fi
done
