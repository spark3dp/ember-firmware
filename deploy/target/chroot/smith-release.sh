#!/bin/sh -e

export LC_ALL=C

#contains: rfs_username, release_date
if [ -f /etc/rcn-ee.conf ] ; then
  . /etc/rcn-ee.conf
fi

if [ -f /etc/oib.project ] ; then
  . /etc/oib.project
fi

setup_system () {
  # Write filesystem release date to dogtag
  echo "Smith Firmware Release Image ${release_date}" > /etc/dogtag

  # Add vim.tiny alias
  echo "alias vim=vim.tiny" >> /etc/bash.bashrc

  # Set the correct path to gem installation directory for shell environment
  echo "export GEM_HOME=/usr/local/lib/gems/1.9.1" >> /etc/profile

  # Create the main storage mount point
  mkdir -p /main

  # Set default run-level to non-graphical
  ln -sfv /lib/systemd/system/multi-user.target /etc/systemd/system/default.target
}

unsecure_root () {
  echo "Log: (chroot) unsecuring root"

  # Clear the root password
  root_password=$(cat /etc/shadow | grep root | awk -F ':' '{print $2}')
  sed -i -e 's:'$root_password'::g' /etc/shadow

  if [ -f /etc/ssh/sshd_config ] ; then
    # Allow ssh login with empty password
    # Make ssh root@beaglebone work
    sed -i -e 's:PermitEmptyPasswords no:PermitEmptyPasswords yes:g' /etc/ssh/sshd_config
    sed -i -e 's:UsePAM yes:UsePAM no:g' /etc/ssh/sshd_config
  fi

  if [ -f /etc/sudoers ] ; then
    #Don't require password for sudo access
    echo "${rfs_username}  ALL=NOPASSWD: ALL" >>/etc/sudoers
  fi
}

setup_startup_scripts () {
  echo "Log: (chroot) setting up startup scripts"
  # Remove the default script put in place by chroot script
  rm -rf /etc/init.d/generic-boot-script.sh

  # Start the cape manager service on boot
  if [ -f /etc/init.d/capemgr.sh ] ; then
    chown root:root /etc/init.d/capemgr.sh
    chown root:root /etc/default/capemgr
    chmod +x /etc/init.d/capemgr.sh
    insserv capemgr.sh || true
  fi

  # systemctl enable returns non-zero exit code (actually the number of symlinks affected) when run in chroot even if successful
  # This is a bug in systemd that appears to have been fixed but the fixed version is not used in debian 7
  # For now, always exit with true

  # Start smith services on boot
  systemctl enable smith.service || true
  systemctl enable smith-server.service || true

  # Enable access point mode on boot
  systemctl enable ap-mode.service || true

  # Restore system date from timestamp file on boot
  systemctl enable restore-date.service || true

  # Start dnsmasq (DHCP server) on boot
  systemctl enable dnsmasq.service || true

  # Remount main storage partition with options from fstab entry on boot
  systemctl enable remount-main-storage.service || true
}

support_readonly() {
  # Makes changes to accomodate read only root
  
  echo "Log: (chroot) updating filesystem to support readonly root"

  # /etc/adjtime is modified on boot, see: https://wiki.debian.org/ReadonlyRoot#adjtime
  ln -s /var/local/adjtime /etc/adjtime

  # /etc/mtab is written to by mount, see: https://wiki.debian.org/ReadonlyRoot#mtab
  ln -s /proc/self/mounts /etc/mtab

  # Generate moddep
  # Need to specify kernel version
  # If kernel version is not specified, depmod uses uname -r to determine the version and in a chroot jail
  # uname -r will return the kernel version of the host system, regardless of what is in the chroot jail
  # The version of the kernel in the chroot jail is known from the config file so use this variable
  # to determine the kernel version passed to depmod
  depmod -a $(echo "${repo_rcnee_pkg_list}" | cut -c13-)

  # Disable remount root filesystem service
  systemctl mask remount-rootfs.service

  # Disable root filesystem check service
  # There is no fsck for squashfs, it is read only
  systemctl mask fsck-root.service

  # Relocate /var/lib/dpkg to /usr/lib since /var isn't included in the firmware image but the dpkg files need to be
  mv -v /var/lib/dpkg /usr/lib/
  ln -sv /usr/lib/dpkg /var/lib/

  # Enable misc services to start on boot
  # Normally this is done automatically by systemd on first boot but this is not possible due to ro filesystem
  systemctl enable acpid.service || true
  systemctl enable wpa_supplicant.service || true

  # Disable service that by default enable units and create symlinks on boot
  # These items are taken care of explicitly
  systemctl mask debian-fixup.service || true
  systemctl mask debian-enable-units.service || true
}

cleanup() {
  # Remove packages installed only for image building process
  apt-get -y --purge remove git git-core git-man sudo
  
  # No need for apt since packages can't be installed on ro filesystem
  dpkg --purge apt
}

setup_system
setup_startup_scripts
unsecure_root
cleanup
support_readonly
