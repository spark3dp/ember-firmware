echo 'Log: in chroot after hook script'
echo 'Log: renaming var'
# Rename var and create an empty directory for use as a mount point
# var resides on the main storage and is not read from the image
mv -v "${tempdir}/var" "${tempdir}/var_contents"
mkdir -v "${tempdir}/var"
