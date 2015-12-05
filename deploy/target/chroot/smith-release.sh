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

# Add vim.tiny alias
echo "alias vim=vim.tiny" >> /etc/bash.bashrc

# Create the main storage mount point
mkdir -p /main

# Start smith firmware on boot
systemctl enable smith.service

# Start smith client on boot
systemctl enable smith-client.service

# Remount main storage partition with options from fstab entry on boot
systemctl enable remount-main-storage.service

# Enable creation of tmp storage if it doesn't exist on boot
systemctl enable create-tmp.service

# Disable remount root filesystem service
systemctl mask remount-rootfs.service

# Disable root filesystem check service
# There is no fsck for squashfs, it is read only
systemctl mask fsck-root.service

# Disable service that by default creates symlinks on boot
# Since the filesystem is read only, the links must exist before packaging the filesystem
# The links needed in /var exist in the var skeleton
systemctl mask debian-fixup.service

# Create the /etc/mtab link that otherwise debian-fixup.service creates
# Also, /etc/mtab is written to by mount, see: https://wiki.debian.org/ReadonlyRoot#mtab
ln -s /proc/self/mounts /etc/mtab

# Generate kernel module dependencies for each supported kernel version
# Normally this is done on first boot but that is not possible due to read-only filesystem
# Need to specify kernel version
# If kernel version is not specified, depmod uses uname -r to determine the version and in a chroot jail
# uname -r will return the kernel version of the host system, regardless of what is in the chroot jail
# The version of the kernel in the chroot jail is known from the config file so use this variable
# to determine the kernel version that is passed to depmod
for _kernel_pkg in $kernel_pkg_list; do
  depmod -a $(echo "${_kernel_pkg}" | cut -c13-)
done

# Remove packages installed only for image building process
apt-get -y --purge remove git git-core git-man sudo

# No need for apt since packages can't be installed on read-only filesystem
dpkg --purge apt

# Relocate /var/lib/dpkg to /usr/lib since /var isn't included in the firmware image but the dpkg files need to be
# The var skeleton contains a symbolic link at /var/lib/dpkg that points to /usr/lib/dpkg
mkdir -p /usr/lib/dpkg
(cd /var/lib/dpkg && tar c .) | (cd /usr/lib/dpkg && tar xf -)
rm -rf /var/lib/dpkg

# Call common functions
configure_readonly
configure_startup_services
unsecure_root
setup_system
