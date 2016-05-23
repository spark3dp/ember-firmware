#!/bin/sh -e

export LC_ALL=C

#contains: rfs_username, release_date
if [ -f /etc/rcn-ee.conf ] ; then
  . /etc/rcn-ee.conf
fi

if [ -f /etc/oib.project ] ; then
  . /etc/oib.project
fi

# Source the common chroot functions
. /common_chroot_functions.sh

echo "Log: (chroot) executing smith-development chroot script"

# Build device tree overlays
# We need BB-I2C2-0A00.dtbo included in the initramfs, otherwise it won't load
dtc_overlays_commit='b11b6e34d68e65afbfc83941c09df77538923665'
cd /opt
git clone https://github.com/beagleboard/bb.org-overlays
cd bb.org-overlays
git checkout "${dtc_overlays_commit}"
./dtc-overlay.sh
make
make install
# Need to specify version of kernel that will end up in the image we generate
update-initramfs -u -k $(echo "${kernel_pkg}" | cut -c13-)
make clean
# remove dtc repository clone
rm -rf ~/git

# Disable rsync service
systemctl disable rsync.service

# Disable one wire server
systemctl disable owserver

# Enlarge the root filesystem on first boot
systemctl enable resize-rootfs.service

# Upgrade/boot AVR firmware on boot
systemctl enable avr-firmware.service

# Install kernel package
dpkg -i "/${kernel_pkg}_1${deb_codename}_${deb_arch}.deb"
rm -v "/${kernel_pkg}_1${deb_codename}_${deb_arch}.deb"

# Install AWS command line interface
pip install awscli

# Call common functions
configure_readonly
configure_startup_services
unsecure_root
setup_system
