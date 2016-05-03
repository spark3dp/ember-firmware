This directory contains files extracted from Debian Jessie initramfs-tools package and changes required by firmware loading scheme.
initramfs-tools contains files from/that would otherwise reside in /usr/share/initramfs-tools.
conf contains files from/that would otherwise reside in /etc/initramfs-tools.
build_initramfs.sh rebuilds the initial RAM disk image used on production printers.
initramfs-tools/init is the init script that loads and starts the firmware image.
