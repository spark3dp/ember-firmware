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

if [ -f /etc/ssh/sshd_config ] ; then
  # Dont print motd and last login on ssh login
  sed -i -e 's:PrintMotd yes:PrintMotd no:g' /etc/ssh/sshd_config
  sed -i -e 's:PrintLastLog yes:PrintLastLog no:g' /etc/ssh/sshd_config
fi

# systemctl enable returns non-zero exit code (actually the number of symlinks affected) when run in chroot even if successful
# This is a bug in systemd that appears to have been fixed but the fixed version is not used in debian 7
# For now, always exit with true

# Disable rsync service
systemctl disable rsync.service || true

# Enlarge the root filesystem on first boot
systemctl enable resize-rootfs.service || true

################################################################################
# Begin configuration for desktop environment
################################################################################
wfile="/etc/lightdm/lightdm.conf"
if [ -f ${wfile} ] ; then
  sed -i -e 's:#autologin-user=:autologin-user='$rfs_username':g' ${wfile}
  sed -i -e 's:#autologin-session=UNIMPLEMENTED:autologin-session='$rfs_default_desktop':g' ${wfile}
  if [ -f /opt/scripts/3rdparty/xinput_calibrator_pointercal.sh ] ; then
    sed -i -e 's:#display-setup-script=:display-setup-script=/opt/scripts/3rdparty/xinput_calibrator_pointercal.sh:g' ${wfile}
  fi
fi

if [ ! "x${rfs_desktop_background}" = "x" ] ; then
  cp -v "${rfs_desktop_background}" /opt/desktop-background.jpg

  mkdir -p /home/${rfs_username}/.config/pcmanfm/LXDE/ || true
  wfile="/home/${rfs_username}/.config/pcmanfm/LXDE/pcmanfm.conf"
  echo "[desktop]" > ${wfile}
  echo "wallpaper_mode=1" >> ${wfile}
  echo "wallpaper=/opt/desktop-background.jpg" >> ${wfile}
  chown -R ${rfs_username}:${rfs_username} /home/${rfs_username}/.config/
fi

#Disable dpms mode and screen blanking
#Better fix for missing cursor
wfile="/home/${rfs_username}/.xsessionrc"
echo "#!/bin/sh" > ${wfile}
echo "" >> ${wfile}
echo "xset -dpms" >> ${wfile}
echo "xset s off" >> ${wfile}
echo "xsetroot -cursor_name left_ptr" >> ${wfile}
chown -R ${rfs_username}:${rfs_username} ${wfile}

#Disable LXDE's screensaver on autostart
if [ -f /etc/xdg/lxsession/LXDE/autostart ] ; then
  cat /etc/xdg/lxsession/LXDE/autostart | grep -v xscreensaver > /tmp/autostart
  mv /tmp/autostart /etc/xdg/lxsession/LXDE/autostart
  rm -rf /tmp/autostart || true
fi

#root password is blank, so remove useless application as it requires a password.
if [ -f /usr/share/applications/gksu.desktop ] ; then
  rm -f /usr/share/applications/gksu.desktop || true
fi

#lxterminal doesnt reference .profile by default, so call via loginshell and start bash
if [ -f /usr/bin/lxterminal ] ; then
  if [ -f /usr/share/applications/lxterminal.desktop ] ; then
    sed -i -e 's:Exec=lxterminal:Exec=lxterminal -l -e bash:g' /usr/share/applications/lxterminal.desktop
    sed -i -e 's:TryExec=lxterminal -l -e bash:TryExec=lxterminal:g' /usr/share/applications/lxterminal.desktop
  fi
fi
################################################################################
# End configuration for desktop environment
################################################################################

# Call common functions
configure_readonly
configure_startup_services
unsecure_root
setup_system
