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
  echo "Log: patching default path"
  # For when sed/grep/etc just gets way to complex...
  cd /
  if [ -f /opt/scripts/mods/debian-add-sbin-usr-sbin-to-default-path.diff ] ; then
    if [ -f /usr/bin/patch ] ; then
      patch -p1 < /opt/scripts/mods/debian-add-sbin-usr-sbin-to-default-path.diff
    fi
  fi

  echo "Log: setting up UART0"
  # Setup UART0
  if [ -f /lib/systemd/system/serial-getty@.service ] ; then
    cp /lib/systemd/system/serial-getty@.service /etc/systemd/system/serial-getty@ttyGS0.service
    ln -s /etc/systemd/system/serial-getty@ttyGS0.service /etc/systemd/system/getty.target.wants/serial-getty@ttyGS0.service

    echo "" >> /etc/securetty
    echo "#USB Gadget Serial Port" >> /etc/securetty
    echo "ttyGS0" >> /etc/securetty
  fi
  
  echo "Smith Firmware Release Image ${release_date}" > /etc/dogtag
}

setup_x () {
  echo "Log: generating .xsessionrc"
  #Disable dpms mode and screen blanking
  #Better fix for missing cursor
  wfile="/home/${rfs_username}/.xsessionrc"
  echo "#!/bin/sh" > ${wfile}
  echo "" >> ${wfile}
  echo "xset -dpms" >> ${wfile}
  echo "xset s off" >> ${wfile}
  echo "xsetroot -cursor_name left_ptr" >> ${wfile}
  chown -R ${rfs_username}:${rfs_username} ${wfile}
}

unsecure_root () {
  echo "Log: unsecuring root"

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
  echo "Log: setting up startup scripts"
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

  # /etc/adjtime is modified on boot, see: https://wiki.debian.org/ReadonlyRoot#adjtime
  ln -s /var/local/adjtime /etc/adjtime

  # Generate moddep
  depmod -a
}

setup_system
setup_x
setup_startup_scripts
support_readonly

unsecure_root
