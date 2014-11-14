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

echo "Log: (chroot) executing smith-release chroot script"

# Write filesystem release date and type to dogtag file
echo "Smith Firmware Release Image ${release_date}" > /etc/dogtag

# Add vim.tiny alias
echo "alias vim=vim.tiny" >> /etc/bash.bashrc

# Create the main storage mount point
mkdir -p /main

# systemctl enable returns non-zero exit code (actually the number of symlinks affected) when run in chroot even if successful
# This is a bug in systemd that appears to have been fixed but the fixed version is not used in debian 7
# For now, always exit with true

# Start smith firmware on boot
systemctl enable smith.service || true

# Start smith client on boot
systemctl enable smith-client.service || true

# Remount main storage partition with options from fstab entry on boot
systemctl enable remount-main-storage.service || true

# Enable creation of tmp storage if it doesn't exist on boot
systemctl enable create-tmp.service || true

# Disable remount root filesystem service
systemctl mask remount-rootfs.service

# Disable root filesystem check service
# There is no fsck for squashfs, it is read only
systemctl mask fsck-root.service

# Generate moddep
# Normally this is done on first boot but that is not possible due to read-only filesystem
# Need to specify kernel version
# If kernel version is not specified, depmod uses uname -r to determine the version and in a chroot jail
# uname -r will return the kernel version of the host system, regardless of what is in the chroot jail
# The version of the kernel in the chroot jail is known from the config file so use this variable
# to determine the kernel version that is passed to depmod
depmod -a $(echo "${repo_rcnee_pkg_list}" | cut -c13-)

# Remove packages installed only for image building process
apt-get -y --purge remove git git-core git-man sudo

# No need for apt since packages can't be installed on read-only filesystem
dpkg --purge apt

# Relocate /var/lib/dpkg to /usr/lib since /var isn't included in the firmware image but the dpkg files need to be
mkdir -p /usr/lib/dpkg
(cd /var/lib/dpkg && tar c .) | (cd /usr/lib/dpkg && tar xf -)
rm -rf /var/lib/dpkg
ln -sv /usr/lib/dpkg /var/lib/

# Call common functions
configure_readonly
configure_startup_services
unsecure_root
setup_system
