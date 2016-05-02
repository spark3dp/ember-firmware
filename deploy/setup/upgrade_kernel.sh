#!/bin/sh -e

setup_dir=$(cd $(dirname "$0"); pwd)
disk=/dev/mmcblk1
var_local_backup_files='smith_server_state smith_state ssid_sufix wpa-roam.conf'

# Display "in progress" message on front panel
upgrade-fp-driver begin

# Make sure any existing partitions are unmounted
umount "${disk}p1" > /dev/null 2>&1 || true
umount "${disk}p2" > /dev/null 2>&1 || true

# Mount eMMC
mkdir -vp /mnt/boot
mkdir -vp /mnt/main
mount "${disk}p1" /mnt/boot
mount "${disk}p2" /mnt/main

# Backup configuration files and user data
rm -rf /root/backup
mkdir -p /root/backup

for file in $var_local_backup_files; do
  if [ -f "/mnt/main/var/local/${file}" ]; then
    cp -v "/mnt/main/var/local/${file}" /root/backup
  fi
done

if [ -f /mnt/main/var/smith/config/settings ]; then
  cp -v /mnt/main/var/smith/config/settings /root/backup
fi

# Remove old firmware, var, and boot files
rm -rvf /mnt/boot/*
rm -fv /mnt/main/firmware/*
rm -rvf /mnt/main/var

# Copy the bootloader files
cp -vr "${setup_dir}/u-boot/"* /mnt/boot

# Copy the initial ram disk image, kernel image, uEnv.txt file, and device tree binary
mkdir -pv /mnt/boot/boot
cp -vr "${setup_dir}/boot/"* /mnt/boot/boot

# Copy the initial firmware image and versions file
cp -v --archive "${setup_dir}/main/firmware" /mnt/main

# Extract the var skeleton
tar xfv "${setup_dir}/main/var.tar" -C /mnt/main

# Restore configuration files
for file in $var_local_backup_files; do
  if [ -f "/root/backup/${file}" ]; then
    cp -v "/root/backup/${file}" /mnt/main/var/local
  fi
done

mkdir -pv /mnt/main/var/smith/config

if [ -f /root/backup/settings ]; then
  cp -v /root/backup/settings /mnt/main/var/smith/config
fi

rm -rfv /root/backup

sync

# Unmount eMMC
umount /mnt/boot
umount /mnt/main
rmdir -v /mnt/boot
rmdir -v /mnt/main

# Display "complete" message on front panel
upgrade-fp-driver complete 
