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

# Write filesystem release date and type to dogtag file
echo "Smith Firmware Development Image ${release_date}" > /etc/dogtag

# Disable rsync service
systemctl disable rsync.service

# Enlarge the root filesystem on first boot
systemctl enable resize-rootfs.service

# Upgrade/boot AVR firmware on boot
systemctl enable avr-firmware.service

# NetBeans fails to detect platform unless the armel version of libc is present
dpkg --add-architecture armel
apt-get update
apt-get -y --force-yes install libc6:armel

# Install newer version of lcov than is available from wheezy repository
dpkg -i /resources/lcov_1.11-2_all.deb

# Install kernel package
dpkg -i "/linux-image-3.8.13-bone71_1${deb_codename}_${deb_arch}.deb"
rm -v "/linux-image-3.8.13-bone71_1${deb_codename}_${deb_arch}.deb"

# Install AWS command line interface
pip install awscli

# Call common functions
configure_readonly
configure_startup_services
unsecure_root
setup_system
