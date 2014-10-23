echo 'Log: in smith-development chroot after hook script'

echo 'Log: removing common chroot functions file from root filesystem directory'
# Remove file copied in before hook
rm -fv "${tempdir}/common_chroot_functions.sh"
