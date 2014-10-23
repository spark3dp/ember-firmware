echo 'Log: in smith-development chroot before hook script'

echo 'Log: installing configuration files'
# Copy common config files
cp -rv "${DIR}/target/config/smith-common/". ${tempdir}
# Copy development specific config files
cp -rv "${DIR}/target/config/smith-development/". ${tempdir}

echo 'Log: copying common chroot functions file to root filesystem directory'
# Copy the common chroot functions so they can be used in the chroot
cp -v "${DIR}/target/scripts/common_chroot_functions.sh" "${tempdir}"
