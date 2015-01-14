# DIR is the deploy directory on the build system
# tempdir is the root directory of the target filesystem

echo 'Log: in smith-development chroot after hook script'

echo 'Log: removing common chroot functions file from root filesystem directory'
# Remove file copied in before hook
rm -fv "${tempdir}/common_chroot_functions.sh"

echo 'Log: restoring resolv.conf'
# Put the original back
mv -v "${tempdir}/etc/resolv.conf.original" "${tempdir}/etc/resolv.conf"

echo 'Log: removing resources'
# Cleanup resources
rm -rfv "${tempdir}/resources"
