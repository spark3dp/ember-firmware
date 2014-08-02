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
  echo "Log: (chroot) patching default path"
  # For when sed/grep/etc just gets way to complex...
  cd /
  if [ -f /opt/scripts/mods/debian-add-sbin-usr-sbin-to-default-path.diff ] ; then
    if [ -f /usr/bin/patch ] ; then
      patch -p1 < /opt/scripts/mods/debian-add-sbin-usr-sbin-to-default-path.diff
    fi
  fi

  echo "Smith Firmware Release Image ${release_date}" > /etc/dogtag

  # Add vim.tiny alias
  echo >> "/etc/bash.bashrc"
  echo -n "alias vim=vim.tiny" >> "/etc/bash.bashrc"

  # Create the main storage mount point
  mkdir -p /main
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

  if [ -f /etc/init.d/smith-boot.sh ] ; then
    chown root:root /etc/init.d/smith-boot.sh
    chmod +x /etc/init.d/smith-boot.sh
    insserv smith-boot.sh || true
  fi

  if [ -f /etc/init.d/capemgr.sh ] ; then
    chown root:root /etc/init.d/capemgr.sh
    chown root:root /etc/default/capemgr
    chmod +x /etc/init.d/capemgr.sh
    insserv capemgr.sh || true
  fi
}

support_readonly() {
  # Makes changes to accomodate read only root
  
  echo "Log: (chroot) updating filesystem to support readonly root"

  # /etc/adjtime is modified on boot, see: https://wiki.debian.org/ReadonlyRoot#adjtime
  ln -s /var/local/adjtime /etc/adjtime

  # /etc/mtab is written to by mount, see: https://wiki.debian.org/ReadonlyRoot#mtab
  ln -s /proc/self/mounts /etc/mtab

  # Generate moddep
  depmod -a

  # Disable file system check on root
  # squashfs does not support fsck
  systemctl mask fsck-root.service

  # Enable umount service to cleanly unmout filesystems on shutdown/reboot
  systemctl enable umount-root.service
}

setup_system
setup_startup_scripts
support_readonly
unsecure_root
