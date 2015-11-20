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

# Disable rsync service
#systemctl disable rsync.service

# Enlarge the root filesystem on first boot
#systemctl enable resize-rootfs.service

# Upgrade/boot AVR firmware on boot
#systemctl enable avr-firmware.service

# Install AWS command line interface
#pip install awscli

# Call common functions
#configure_readonly
configure_startup_services
unsecure_root
#setup_system
