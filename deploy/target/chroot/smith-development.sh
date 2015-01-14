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

# systemctl enable returns non-zero exit code (actually the number of symlinks affected) when run in chroot even if successful
# This is a bug in systemd that appears to have been fixed but the fixed version is not used in debian 7
# For now, always exit with true

# Disable rsync service
systemctl disable rsync.service || true

# Enlarge the root filesystem on first boot
systemctl enable resize-rootfs.service || true

# NetBeans fails to detect platform unless the armel version of libc is present
dpkg --add-architecture armel
apt-get update
apt-get -y --force-yes install libc6:armel

# Install newer version of lcov than is available from wheezy repository
dpkg -i /resources/lcov_1.11-2_all.deb

# Call common functions
configure_readonly
configure_startup_services
unsecure_root
setup_system
