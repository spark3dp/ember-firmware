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
