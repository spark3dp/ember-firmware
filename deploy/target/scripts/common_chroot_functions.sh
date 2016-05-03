# Support the read-only configuration that
# is shared between development and release filesystems
# Only the release filesystem is actually read-only but as much as possible
# of the configuration is shared with the development filesystem
# This function makes the corresponding changes in the chroot jail to support
# the shared configuration
configure_readonly() {
  echo "Log: (chroot) updating configuration to support shared read-only configuration"

  # Set the correct path to gem installation directory for shell environment
  echo "export GEM_HOME=/usr/local/lib/gems/2.1.0" >> /etc/profile

  # Remove the update script used to communicate DNS updates from resolvconf to dnsmasq
  # We don't use the DNS server portion of dnsmasq
  # If the script exists on the release filesystem, then it tries to create a directory
  # in /var/run before the /var directory has been loop-back mounted to the main (rw) storage
  # Deleting this script prevents failed attempts to create the directory
  # Another approach may involve setting up ordering between the resolvconf systemd unit and
  # the local-fs.target but that would require editing the unit file provided by the resolvconf package
  if [ -f /etc/resolvconf/update.d/dnsmasq ]; then
    rm /etc/resolvconf/update.d/dnsmasq
  fi
}

# Configure various services to start on boot
configure_startup_services() {
  echo "Log: (chroot) configuring common startup services"

  # Don't use generic board startup script or capemgr
  # Use fine-grain systemd startup tasks in place of the monolithic generic board startup script
  # We don't seem to need the capemgr

  if [ -f /lib/systemd/system/generic-board-startup.service ]; then
    systemctl disable generic-board-startup.service
    rm /lib/systemd/system/generic-board-startup.service
  fi

  if [ -f /lib/systemd/system/capemgr.service ]; then
    systemctl disable capemgr.service
    rm /lib/systemd/system/capemgr.service
  fi

  # Start the local web server on boot
  systemctl enable smith-server.service

  # Enable access point mode on boot
  systemctl enable initialize-network.service

  # Start dnsmasq (DHCP server) on boot
  systemctl enable dnsmasq.service

  # Mount owfs on boot
  systemctl enable owfs.service

  # Start wpa_supplicant on boot 
  systemctl enable wpa_supplicant.service
  
  # Enable service to close ssh sessions on reboot/poweroff
  systemctl enable kill-ssh-sessions.service
}

# Allow root login over ssh and remove the root password
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
    sed -i -e 's:PermitRootLogin without-password:PermitRootLogin yes:g' /etc/ssh/sshd_config
  fi

  if [ -f /etc/sudoers ] ; then
    #Don't require password for sudo access
    echo "${rfs_username}  ALL=NOPASSWD: ALL" >>/etc/sudoers
  fi
}

# Miscellaneous system-level setup tasks
setup_system() {
  # Set default run-level to non-graphical
  ln -sfv /lib/systemd/system/multi-user.target /etc/systemd/system/default.target

  # Create the mount point for owfs
  mkdir -pv /mnt/1wire

  # Create the mount point for usb drives
  mkdir -pv /mnt/usb

  if [ -f /etc/ssh/sshd_config ]; then
    # Dont print motd and last login on ssh login
    sed -i -e 's:PrintMotd yes:PrintMotd no:g' /etc/ssh/sshd_config
    sed -i -e 's:PrintLastLog yes:PrintLastLog no:g' /etc/ssh/sshd_config
  fi

  # Eliminate configuration causing rsyslog to produce excessive log output
  # See https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=742113
  if [ -f /etc/rsyslog.conf ]; then
    sed -i '/daemon\.\*;mail\.\*;\\/,+3 s/^/#/' /etc/rsyslog.conf
  fi

  # Update ldconfig cache so programs can locate manually added shared libraries
  ldconfig
}
