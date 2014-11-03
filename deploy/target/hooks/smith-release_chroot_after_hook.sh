# DIR is the deploy directory on the build system
# tempdir is the root directory of the target filesystem

echo 'Log: in smith-release chroot after hook script'

echo 'Log: renaming var'
# Rename var and create an empty directory for use as a mount point
# var resides on the main storage and is not read from the image
mv -v "${tempdir}/var" "${tempdir}/var_contents"
mkdir -v "${tempdir}/var"

echo 'Log: removing common chroot functions file from root filesystem directory'
# Remove file copied in before hook
rm -fv "${tempdir}/common_chroot_functions.sh"
